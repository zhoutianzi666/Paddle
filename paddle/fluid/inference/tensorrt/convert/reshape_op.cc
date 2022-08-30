/* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/fluid/inference/tensorrt/convert/op_converter.h"

namespace paddle {
namespace framework {
class Scope;
namespace proto {
class OpDesc;
}  // namespace proto
}  // namespace framework
}  // namespace paddle

namespace paddle {
namespace inference {
namespace tensorrt {

/*
 * ReshapeOp
 */
class ReshapeOpConverter : public OpConverter {
 public:
  void operator()(const framework::proto::OpDesc& op,
                  const framework::Scope& scope,
                  bool test_mode) override {
    framework::OpDesc op_desc(op, nullptr);
    // Declare inputs
    auto* input = engine_->GetITensor(op_desc.Input("X")[0], scope);

    std::vector<int> shape =
        PADDLE_GET_CONST(std::vector<int>, op_desc.GetAttr("shape"));
    int nbDims_num = shape.size();
    nvinfer1::Dims reshape_dim;
    nvinfer1::ITensor* real_shape_tensor = nullptr;
    std::vector<nvinfer1::ITensor*> concat_inputs;
    bool one_input = false;
    if (engine_->with_dynamic_shape()) {
      if (op_desc.Inputs().find("ShapeTensor") != op_desc.Inputs().end() &&
          op_desc.Input("ShapeTensor").size() > 0) {
        for (auto name : op_desc.Input("ShapeTensor"))
          concat_inputs.push_back(engine_->GetITensor(name, scope));
        real_shape_tensor = Concat(concat_inputs);
      } else if (op_desc.Inputs().find("Shape") != op_desc.Inputs().end() &&
                 op_desc.Input("Shape").size() > 0) {
        real_shape_tensor = engine_->GetITensor(op_desc.Input("Shape")[0], scope);
      } else {
        reshape_dim.nbDims = nbDims_num;
        for (int i = 0; i < nbDims_num; ++i) {
          reshape_dim.d[i] = shape[i];
        }
        one_input = true;
      }
    } else {  // running the TRT Static Shape mode
      reshape_dim.nbDims = nbDims_num - 1;
      for (int i = 0; i < nbDims_num - 1; ++i) {
        reshape_dim.d[i] = shape[i + 1];
      }
    }
    auto* layer = TRT_ENGINE_ADD_LAYER(engine_, Shuffle, *input);
    if (!engine_->with_dynamic_shape() || one_input)
      layer->setReshapeDimensions(reshape_dim);
    else
      layer->setInput(1, *real_shape_tensor);
    auto output_name = op_desc.Output("Out")[0];
    RreplenishLayerAndOutput(layer, "reshape", {output_name}, test_mode);
  }
};

}  // namespace tensorrt
}  // namespace inference
}  // namespace paddle

REGISTER_TRT_OP_CONVERTER(reshape, ReshapeOpConverter);
REGISTER_TRT_OP_CONVERTER(reshape2, ReshapeOpConverter);
