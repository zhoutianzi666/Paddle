// Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.
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

#include "paddle/phi/kernels/temporal_shift_kernel.h"

#include "paddle/common/layout.h"
#include "paddle/phi/backends/cpu/cpu_context.h"
#include "paddle/phi/core/kernel_registry.h"

namespace phi {

template <typename T>
void TemporalShiftFwNCHW(const T* input,
                         T* output,
                         const int ntchw,
                         const int tchw,
                         const int chw,
                         const int hw,
                         const int t,
                         const int c1,
                         const int c2) {
  int src_it = 0;
  for (int i = 0; i < ntchw; i++) {
    int it = (i % tchw) / chw;
    int ic = (i % chw) / hw;

    if (ic < c1) {
      src_it = it - 1;
    } else if (ic < c2) {
      src_it = it + 1;
    } else {
      src_it = it;
    }

    if (src_it < 0 || src_it >= t) {
      output[i] = 0;
    } else {
      output[i] = input[i + (src_it - it) * chw];
    }
  }
}

template <typename T>
void TemporalShiftFwNHWC(const T* input,
                         T* output,
                         const int nthwc,
                         const int thwc,
                         const int hwc,
                         const int t,
                         const int c,
                         const int c1,
                         const int c2) {
  int src_it = 0;
  for (int i = 0; i < nthwc; i++) {
    int it = (i % thwc) / hwc;
    int ic = i % c;

    if (ic < c1) {
      src_it = it - 1;
    } else if (ic < c2) {
      src_it = it + 1;
    } else {
      src_it = it;
    }

    if (src_it < 0 || src_it >= t) {
      output[i] = 0;
    } else {
      output[i] = input[i + (src_it - it) * hwc];
    }
  }
}

template <typename T, typename Context>
void TemporalShiftKernel(const Context& dev_ctx,
                         const DenseTensor& x,
                         int seg_num,
                         float shift_ratio,
                         const std::string& data_format_str,
                         DenseTensor* out) {
  auto* input = &x;
  auto* output = out;
  int t = seg_num;
  const DataLayout data_layout = common::StringToDataLayout(data_format_str);

  const int nt = static_cast<int>(input->dims()[0]);
  const int c = static_cast<int>(
      data_layout == DataLayout::kNCHW ? input->dims()[1] : input->dims()[3]);
  const int h = static_cast<int>(
      data_layout == DataLayout::kNCHW ? input->dims()[2] : input->dims()[1]);
  const int w = static_cast<int>(
      data_layout == DataLayout::kNCHW ? input->dims()[3] : input->dims()[2]);

  const int hw = h * w;
  const int chw = c * hw;
  const int tchw = t * chw;
  const int ntchw = nt * chw;

  const int c1 = static_cast<int>(static_cast<float>(c) * shift_ratio);
  const int c2 = static_cast<int>(static_cast<float>(c) * 2.f * shift_ratio);

  DDim out_dims =
      (data_layout == DataLayout::kNCHW ? common::make_ddim({nt, c, h, w})
                                        : common::make_ddim({nt, h, w, c}));
  const T* input_data = input->data<T>();
  output->Resize(out_dims);
  T* output_data = dev_ctx.template Alloc<T>(output);

  if (data_layout == DataLayout::kNCHW) {
    TemporalShiftFwNCHW<T>(
        input_data, output_data, ntchw, tchw, chw, hw, t, c1, c2);
  } else {
    TemporalShiftFwNHWC<T>(
        input_data, output_data, ntchw, tchw, chw, t, c, c1, c2);
  }
}

}  // namespace phi

PD_REGISTER_KERNEL(
    temporal_shift, CPU, ALL_LAYOUT, phi::TemporalShiftKernel, float, double) {}
