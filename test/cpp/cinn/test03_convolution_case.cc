// Copyright (c) 2021 CINN Authors. All Rights Reserved.
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

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "paddle/cinn/runtime/cinn_runtime.h"
#include "paddle/cinn/utils/timer.h"
#include "test/cpp/cinn/test03_convolution.h"

TEST(test03, basic) {}

// include the generated C source code:
// @{
#include "test/cpp/cinn/test03_convolution.cc"
// @}
