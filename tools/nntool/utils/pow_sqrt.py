# Copyright (C) 2020  GreenWaves Technologies, SAS

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# square root coefficients in 17.15 fixed point format

import logging

import numpy as np

from utils.exp_17_15 import (exp_fp_17_15, gap_roundnorm,
                             gap_roundnorm_reg)

LOG = logging.getLogger('nntool.'+__name__)

SqrtCoeffTable = np.array([
    0x00001A91, 0x0000BA3A, 0xFFFF53DA, 0x00008DAC,
    0xFFFFBB54, 0x00000E5A
], dtype=np.int32)

# square root of 2 powered to(index - 15) in 17.15 format

Sqrt2Powers = np.array([
    0x000000B5,	0x00000100, 0x0000016A, 0x00000200,
    0x000002D4,	0x00000400, 0x000005A8, 0x00000800,
    0x00000B50,	0x00001000, 0x000016A1, 0x00002000,
    0x00002D41,	0x00004000, 0x00005A82, 0x00008000,
    0x0000B505,	0x00010000, 0x00016A0A, 0x00020000,
    0x0002D414,	0x00040000, 0x0005A828, 0x00080000,
    0x000B504F,	0x00100000, 0x0016A09E, 0x00200000,
    0x002D413D,	0x00400000, 0x005A827A, 0x00800000
], dtype=np.int32)

# log coefficients in 17.15 fixed point format

LN_2_1F15 = 0x000058B9
LN_10_INV_Q10 = 0x000001bd
LOG10_2 = 0x00002688

LognCoeffTable = np.array([
    0x00007FE3, 0xFFFFC149, 0x00002491, 0xFFFFEEF8,
    0x00000404
], dtype=np.int32)

PI_Q17_15 = 102944 # int(math.floor(0.5 + math.pi * math.pow(2, 15)))
PI_Q17_15_DIV4 = 25736 # int(math.floor(0.5 + PI_Q17_15 / 4))
PI_Q1_30_DIV4 = 843314857 # int(math.floor(0.5 + math.pi * math.pow(2, 30) / 4))
ARCTAN_FAC_Q17_15 = 8946 # int(math.floor(0.5 + 0.273 * math.pow(2, 15)))
ONE_Q17_15 = 32768 # 1 << 15

def gap_clb(x):
    '''Count leading redundant sign bits'''
    def func(x):
        x = np.binary_repr(x, width=32)
        return np.int32(len(x) - len(x.lstrip(x[0])) - 1)
    x = np.atleast_1d(x).astype(np.uint32)
    return np.vectorize(func)(x)

def gap_fl1(x):
    '''Position of the most significant bit'''
    def func(x):
        x = np.binary_repr(x, width=32)
        clz = (len(x) - len(x.lstrip('0')))
        return (31 - clz) if clz < 32 else 32
    x = np.atleast_1d(x).astype(np.uint32)
    return np.vectorize(func)(x)

def emul_clz(x):
    '''clz with gap_fl1 - note that clz is undefined for 0'''
    if np.any(x==0):
        raise ValueError('clz is undefined for 0')
    return (31 - gap_fl1(x)).astype(np.int32)

def arctan_17_15(x):
    """Valid for 1 > x > -1"""
    # This can use p.mulsRN and p.adduRN on GAP so 5 cycles
    # p.adduRN(PI_Q17_15_DIV4 * x, ARCTAN_FAC_Q17_15 * p.mulsRN(x * (ONE_Q17_15 - x))
    return gap_roundnorm(PI_Q17_15_DIV4 * x + ARCTAN_FAC_Q17_15 * gap_roundnorm(x * (ONE_Q17_15 - x), 15), 15)

def arctan_17_15alt(x):
    """Valid for 1 > x > -1"""
    # This can use p.mulsRN and p.adduRN on GAP so 4 cycles
    # p.mulsRN(x, p.adduRN(PI_Q1_30_DIV4, ARCTAN_FAC_Q17_15 * (ONE_Q17_15 - x)))
    return gap_roundnorm_reg(x * gap_roundnorm_reg(PI_Q1_30_DIV4 + ARCTAN_FAC_Q17_15 * (ONE_Q17_15 - x), 15), 15)

def sqrt_17_15(x, is_unsigned=False):
    x = x.astype(np.uint32)
    if not x.shape:
        x = x.reshape([1])
    if is_unsigned:
        mask = x != 0
    else:
        mask = np.logical_and(x != 0, x <= 0x7FFFFFFF)
    result = np.zeros_like(x, dtype=np.uint32)

    exponent = np.array(gap_clb(x), dtype=np.uint32).reshape(x.shape)
    y = np.array(((x << exponent) >> 16)).astype(np.uint32)

    # sqrt(x) = 0.2075806 + 1.454895 * x - 1.34491 * x^2 + 1.106812 * x^3 - 0.536499 * x^4 + 0.1121216 * x^5
    z = y.copy()

    for elem in SqrtCoeffTable[1::]:
        result[mask] = np.int32(result[mask]) + np.int32(z[mask] * elem)
        z[mask] = ((z[mask] * y[mask]) >> 15)

    result[mask] >>= 15
    result[mask] += SqrtCoeffTable[0]
    result[mask] = np.where(exponent[mask] != 16,
                            np.where(exponent[mask] >=12, 
                                     (result[mask] * Sqrt2Powers[31 - exponent[mask]]) >> 15,
                                     ((result[mask]>>(12-exponent[mask])) * Sqrt2Powers[31 - exponent[mask]]) >> (15-(12-exponent[mask]))),
                            result[mask])
    return result.astype(np.uint32)


def logn_17_15(x, is_unsigned=False):

        # register uint32_t       i, exponent;
        # register int32_t        result, y, z;

    x = x.astype(np.uint32)
    if not x.shape:
        x = x.reshape([1])
    if is_unsigned:
        mask = x != 0
    else:
        mask = np.logical_and(x != 0, x <= 0x7FFFFFFF)
    result = np.zeros_like(x, dtype=np.int32)
    result[np.logical_not(mask)] = 0x80000000

    exponent = np.array(gap_clb(x), dtype=np.uint32).reshape(x.shape)
    y = (((x << exponent) >> 15) - 0x8000).astype(np.int32)

    #  ln(x) = 0.9991150 * (x -1) - 0.4899597 * (x -1) ^ 2
    #        + 0.2856751 * (x -1) ^ 3 - 0.1330566 * (x -1) ^ 4
    #        + 0.03137207 * (x -1) ^ 5

    z = ((y * y) >> 15)
    result[mask] = LognCoeffTable[0] * y[mask]
    for elem in LognCoeffTable[1::]:
        result[mask] += z[mask] * elem
        z[mask] = ((z[mask] * y[mask]) >> 15)

    result[mask] >>= 15
    result[mask] = np.where(exponent[mask] != 15, result[mask] +
                            LN_2_1F15 * (15 - exponent[mask]), result[mask])

    return result

# x in Q17.15 y in Q0
def int_pow_17_15(x, y):
    if isinstance(y, int):
        n = np.full_like(x, y)
    elif y.size == 1 and x.size > 1:
        n = np.full_like(x, y[0])
    else:
        n = y.copy()
    res = np.ones_like(x) << 15
    factor = x.copy().astype(np.int32)
    factor_mult = gap_roundnorm(factor, 7)
    while True:
        mask = np.logical_and(n > 0, factor > 0)
        if not np.any(mask):
            break
        res[mask] = np.where(n[mask] & 1 != 0, gap_roundnorm(np.multiply(res[mask], factor[mask], dtype=np.int32), 15), res[mask])
        factor[mask] = gap_roundnorm(np.multiply(factor[mask], factor_mult[mask], dtype=np.int32), 8)
        n[mask] >>= 1
    return res

# x in Q0.15 y in Q17_15
def pow_alt_17_15(x, y):
    if isinstance(y, int):
        y = np.full_like(x, y)
    elif y.size == 1 and x.size > 1:
        y = np.full_like(x, y[0])
    yneg_mask = y < 0
    exponent = np.abs(y)
    res = gap_roundnorm(int_pow_17_15(x, exponent>>15) * pow_17_15(x, y&0x7fff), 15)
    res[yneg_mask] = gap_roundnorm((1 << 15) / res[yneg_mask], 15)
    return res

# a**x = e**(x*ln(a))
def pow_17_15(x, y):
    x = np.atleast_1d(x)
    y = np.atleast_1d(y)
    clip_bits = gap_clb(y)
    limit_high = (1 << clip_bits) - 1
    limit_low = -(1 << clip_bits)
    if isinstance(y, int):
        y = np.full_like(x, y)
    elif y.size == 1 and x.size > 1:
        y = np.full_like(x, y[0])
    assert np.all(y >= 0), "only postive exponents currently supported"
    assert np.all(y >> 15 == 0), "only fractional exponents currently supported"
    return np.where(x == 0, np.where(y == 0, np.int32(1<<15), np.int32(0)), exp_fp_17_15(gap_roundnorm(y * np.clip(logn_17_15(x), limit_low, limit_high), 15))).astype(np.int32)

def square_17_15(x):
    return gap_roundnorm_reg(x * x, 15)

RSQRT_TAB = np.array([
    0xfa0bdefa, 0xee6af6ee, 0xe5effae5, 0xdaf27ad9,
    0xd2eff6d0, 0xc890aec4, 0xc10366bb, 0xb9a71ab2,
    0xb4da2eac, 0xadce7ea3, 0xa6f2b29a, 0xa279a694,
    0x9beb568b, 0x97a5c685, 0x9163027c, 0x8d4fd276,
    0x89501e70, 0x8563da6a, 0x818ac664, 0x7dc4fe5e,
    0x7a122258, 0x7671be52, 0x72e44a4c, 0x6f68fa46,
    0x6db22a43, 0x6a52623d, 0x67041a37, 0x65639634,
    0x622ffe2e, 0x609cba2b, 0x5d837e25, 0x5bfcfe22,
    0x58fd461c, 0x57838619, 0x560e1216, 0x53300a10,
    0x51c72e0d, 0x50621a0a, 0x4da48204, 0x4c4c2e01,
    0x4af789fe, 0x49a689fb, 0x485a11f8, 0x4710f9f5,
    0x45cc2df2, 0x448b4def, 0x421505e9, 0x40df5de6,
    0x3fadc5e3, 0x3e7fe1e0, 0x3d55c9dd, 0x3d55d9dd,
    0x3c2f41da, 0x39edd9d4, 0x39edc1d4, 0x38d281d1,
    0x37bae1ce, 0x36a6c1cb, 0x3595d5c8, 0x3488f1c5,
    0x3488fdc5, 0x337fbdc2, 0x3279ddbf, 0x317749bc,
    0x307831b9, 0x307879b9, 0x2f7d01b6, 0x2e84ddb3,
    0x2d9005b0, 0x2d9015b0, 0x2c9ec1ad, 0x2bb0a1aa,
    0x2bb0f5aa, 0x2ac615a7, 0x29ded1a4, 0x29dec9a4,
    0x28fabda1, 0x2819e99e, 0x2819ed9e, 0x273c3d9b,
    0x273c359b, 0x2661dd98, 0x258ad195, 0x258af195,
    0x24b71192, 0x24b6b192, 0x23e6058f, 0x2318118c,
    0x2318718c, 0x224da189, 0x224dd989, 0x21860d86,
    0x21862586, 0x20c19183, 0x20c1b183, 0x20001580
], dtype=np.uint32)


def umulhi(x, y):
    return ((x.astype(np.uint64) * y.astype(np.uint64))>>32).astype(np.uint32)

# Q16.16 RSQRT
def rsqrt_16_16(x):
    mask = x==0
    x = np.atleast_1d(x).astype(np.uint32)
    scal = emul_clz(x).astype(np.uint32) & 0xfffffffe
    y = x << scal
    idx = (y >> 25) - 32
    first_approx = RSQRT_TAB [idx]
    res = (first_approx << 22) - umulhi (first_approx, y);
    factor = umulhi (res, y)
    factor = 0x30000000 - umulhi (res, factor)
    res = umulhi (res, factor)
    res = ((res >> (18 - (scal >> 1))) + 1) >> 1
    res[mask] = ~x[mask]
    return res