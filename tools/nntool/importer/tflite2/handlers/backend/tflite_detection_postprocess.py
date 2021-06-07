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

import numpy as np
from graph.types import NNEdge, SSDDetectorParameters
from importer.tflite2.common.tflite_node import TFLiteNode
from importer.tflite2.common.tflite_tensor import TensorBase
from quantization.new_qrec import QRec
from quantization.qtype import QType
from utils.node_id import NodeId

from ..backend_handler import BackendHandler
from ..handler import tflite_custom_op, tflite_op


@tflite_op("CUSTOM")
@tflite_custom_op("TFLite_Detection_PostProcess")
class TFLiteDetectionPostProcess(BackendHandler):

    @classmethod
    def _common(cls, node: TFLiteNode, **kwargs):
        custom_opts = node.get_custom_options()
        G = kwargs['G']
        opts = kwargs['opts']
        all_nodes = kwargs['all_nodes']
        importer = kwargs['importer']

        inputs = [all_nodes[t] for t in node.input]
        outputs = [all_nodes.get(node.output[idx]) if idx < len(node.output) else None
                   for idx in range(4)]
        # inp_shapes = [input[2].shape for input in inputs]

        if 'max_bb_before_nms' not in custom_opts:
            custom_opts['max_bb_before_nms'] = 300

        params = SSDDetectorParameters(node.name,
                                       parameters=custom_opts)

        overriden_outputs = []
        for idx, output in enumerate(outputs):
            if output:
                overriden_outputs.append(node.output[idx])
                continue
            oparams = G.add_output()
            otensor = TensorBase("Detect_%s" % idx)
            overriden_outputs.append(otensor)
            importer.provisional_outputs[otensor] = (oparams, 0, None)
        # covers the case where not all outputs are generated by the conversion tool
        node.override_outputs(overriden_outputs)

        for idx, inp in enumerate(inputs):
            G.add_edge(
                NNEdge(from_node=inp[0], to_node=params, from_idx=inp[1], to_idx=idx))

        if opts.get('load_quantization'):
            in_qtypes = [QType.from_min_max_sq(tensor.qtype.min_val, tensor.qtype.max_val)
                         if (tensor.qtype.is_asymmetric or not tensor.qtype.signed) else tensor.qtype
                         for tensor in node.input]
            o_boxes_qtype = QType(min_val=-2, max_val=2,
                                  dtype=np.int16, scale=2**(-14))
            o_scores_qtype = node.input[1].qtype
            o_class_qtype = QType(scale=1, dtype=np.int8)
            qrec = QRec.scaled(in_qs=in_qtypes,
                               out_qs=[o_boxes_qtype, o_class_qtype,
                                       o_scores_qtype, o_class_qtype])
            G.quantization[NodeId(params)] = qrec

        return params

    @classmethod
    def version_1(cls, node: TFLiteNode, **kwargs):
        return cls._common(node, **kwargs)
