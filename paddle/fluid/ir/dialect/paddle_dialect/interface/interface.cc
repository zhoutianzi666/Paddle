// Copyright (c) 2023 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "paddle/fluid/ir/dialect/paddle_dialect/interface/infermeta.h"
#include "paddle/fluid/ir/dialect/paddle_dialect/interface/op_yaml_info.h"
#include "paddle/fluid/ir/dialect/paddle_dialect/interface/vjp.h"

IR_DEFINE_EXPLICIT_TYPE_ID(paddle::dialect::InferMetaInterface)
IR_DEFINE_EXPLICIT_TYPE_ID(paddle::dialect::OpYamlInfoInterface)
IR_DEFINE_EXPLICIT_TYPE_ID(paddle::dialect::VjpInterface)
