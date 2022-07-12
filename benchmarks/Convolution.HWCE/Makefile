# Copyright 2019 GreenWaves Technologies, SAS
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

PULP_APP = Convolution.HWCE
TILER_PATH = $(GAP_SDK_HOME)/tools/autotiler_v2
TILER_GENERATOR_PATH = $(TILER_PATH)/generators
MNIST_KER_PATH = $(TILER_GENERATOR_PATH)/CNN/kernels

PULP_APP_SRCS = convolution.hwce.c
PULP_APP_SRCS += $(MNIST_KER_PATH)/src/CNN_HwCE.c

pulpFc=1
RM=rm -f
NB_FRAMES ?= -1

PULP_LDFLAGS +=  -flto -Wl,--gc-sections

PULP_CFLAGS += -I$(MNIST_KER_PATH)/include -DNB_FRAMES=$(NB_FRAMES)
PULP_CFLAGS += -O3 -DRT_HAS_HWCE=1 
PULP_CFLAGS += -mno-memcpy -fno-tree-loop-distribute-patterns  -fdata-sections -ffunction-sections
PULP_CFLAGS += -Wall -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function


include $(GAP_SDK_HOME)/tools/rules/pulp_rules.mk
