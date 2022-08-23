# Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.
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

from trt_layer_auto_scan_test import TrtLayerAutoScanTest, SkipReasons
from program_config import TensorConfig, ProgramConfig
import unittest
import numpy as np
import paddle.inference as paddle_infer
from functools import partial
from typing import Optional, List, Callable, Dict, Any, Set


def generate_input(type):
    if type == 0:
        return np.ones([1, 3, 64, 64]).astype(np.bool)
    elif type == 2:
        return np.ones([1, 3, 64, 64]).astype(np.int32)
    elif type == 4:
        return np.ones([1, 3, 64, 64]).astype(np.float16)
    else:
        return np.ones([1, 3, 64, 64]).astype(np.float32)


def dtype2int(type):
    if type == "bool":
        return 0
    elif type == "int":
        return 2
    elif type == "fp16":
        return 4
    elif type == "fp32":
        return 5


class TrtConvertCastTest(TrtLayerAutoScanTest):

    def sample_program_configs(self):

        for [in_dtype, out_dtype] in [
            ["bool", "int"],
            ["bool", "fp32"],
            ["int", "fp32"],
            ["int", "int"],
            ["fp32", "int"],
            ["fp32", "fp32"],
        ]:
            dics = [{
                "in_dtype": dtype2int(in_dtype),
                "out_dtype": dtype2int(out_dtype)
            }]
            ops_config = [
                {
                    "op_type": "cast",
                    "op_inputs": {
                        "X": ["input_data"]
                    },
                    "op_outputs": {
                        "Out": ["cast_output_data"]
                    },
                    "op_attrs": dics[0]
                },
                {
                    "op_type": "elementwise_add",
                    "op_inputs": {
                        "X": ["cast_output_data"],
                        "Y": ["cast_output_data"]
                    },
                    "op_outputs": {
                        "Out": ["output_data"]
                    },
                    "op_attrs": {
                        "axis": -1
                    },
                },
            ]
            ops = self.generate_op_config(ops_config)

            program_config = ProgramConfig(
                ops=ops,
                weights={},
                inputs={
                    "input_data":
                    TensorConfig(data_gen=partial(generate_input, in_dtype))
                },
                outputs=["output_data"])

            yield program_config

    def sample_predictor_configs(
            self, program_config) -> (paddle_infer.Config, List[int], float):

        def generate_dynamic_shape(attrs):
            self.dynamic_shape.min_input_shape = {"input_data": [1, 3, 64, 64]}
            self.dynamic_shape.max_input_shape = {"input_data": [4, 3, 64, 64]}
            self.dynamic_shape.opt_input_shape = {"input_data": [1, 3, 64, 64]}

        def clear_dynamic_shape():
            self.dynamic_shape.min_input_shape = {}
            self.dynamic_shape.max_input_shape = {}
            self.dynamic_shape.opt_input_shape = {}

        def generate_trt_nodes_num(attrs, dynamic_shape):
            return 1, 2

        attrs = [
            program_config.ops[i].attrs for i in range(len(program_config.ops))
        ]

        # for static_shape
        clear_dynamic_shape()
        self.trt_param.precision = paddle_infer.PrecisionType.Float32
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, False), 1e-5
        self.trt_param.precision = paddle_infer.PrecisionType.Half
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, False), 1e-2

        # for dynamic_shape
        generate_dynamic_shape(attrs)
        self.trt_param.precision = paddle_infer.PrecisionType.Float32
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, True), 1e-5
        self.trt_param.precision = paddle_infer.PrecisionType.Half
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, True), 1e-2

    def test(self):
        self.run_test()


class TrtConvertCastTest2(TrtLayerAutoScanTest):

    def sample_program_configs(self):
        for [in_dtype, out_dtype] in [
            ["int", "bool"],
            ["fp32", "bool"],
        ]:
            dics = [
                {
                    "in_dtype": dtype2int(in_dtype),
                    "out_dtype": dtype2int(out_dtype)
                },
                {
                    "in_dtype": dtype2int(out_dtype),
                    "out_dtype": dtype2int(in_dtype)
                },
            ]
            ops_config = [
                {
                    "op_type": "cast",
                    "op_inputs": {
                        "X": ["input_data"]
                    },
                    "op_outputs": {
                        "Out": ["cast_output_data"]
                    },
                    "op_attrs": dics[0]
                },
                {
                    "op_type": "cast",
                    "op_inputs": {
                        "X": ["cast_output_data"],
                    },
                    "op_outputs": {
                        "Out": ["output_data0"]
                    },
                    "op_attrs": dics[1]
                },
                {
                    "op_type": "elementwise_add",
                    "op_inputs": {
                        "X": ["output_data0"],
                        "Y": ["output_data0"]
                    },
                    "op_outputs": {
                        "Out": ["output_data1"]
                    },
                    "op_attrs": {
                        "axis": -1
                    },
                },
            ]
            ops = self.generate_op_config(ops_config)

            program_config = ProgramConfig(
                ops=ops,
                weights={},
                inputs={
                    "input_data":
                    TensorConfig(data_gen=partial(generate_input, in_dtype))
                },
                outputs=["output_data1"])

            yield program_config

    def sample_predictor_configs(
            self, program_config) -> (paddle_infer.Config, List[int], float):
        self.trt_param.workspace_size = 1073741824

        def generate_dynamic_shape(attrs):
            self.dynamic_shape.min_input_shape = {"input_data": [1, 3, 64, 64]}
            self.dynamic_shape.max_input_shape = {"input_data": [4, 3, 64, 64]}
            self.dynamic_shape.opt_input_shape = {"input_data": [1, 3, 64, 64]}

        def clear_dynamic_shape():
            self.dynamic_shape.min_input_shape = {}
            self.dynamic_shape.max_input_shape = {}
            self.dynamic_shape.opt_input_shape = {}

        def generate_trt_nodes_num(attrs, dynamic_shape):
            return 1, 2

        attrs = [
            program_config.ops[i].attrs for i in range(len(program_config.ops))
        ]

        # for static_shape
        clear_dynamic_shape()
        self.trt_param.precision = paddle_infer.PrecisionType.Float32
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, False), 1e-5
        self.trt_param.precision = paddle_infer.PrecisionType.Half
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, False), 1e-2

        # for dynamic_shape
        generate_dynamic_shape(attrs)
        self.trt_param.precision = paddle_infer.PrecisionType.Float32
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, True), 1e-5
        self.trt_param.precision = paddle_infer.PrecisionType.Half
        yield self.create_inference_config(), generate_trt_nodes_num(
            attrs, True), 1e-2

    def test(self):
        self.run_test()


if __name__ == "__main__":
    unittest.main()
