// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <vector>

#include "fully_connected_kernel_fb_io_ref.h"

namespace kernel_selector {
ParamsKey FullyConnected_fb_io_ref::GetSupportedKey() const {
    ParamsKey k;
    k.EnableInputDataType(Datatype::F16);
    k.EnableInputDataType(Datatype::F32);
    k.EnableOutputDataType(Datatype::F16);
    k.EnableOutputDataType(Datatype::F32);
    k.EnableInputWeightsType(WeightsType::F16);
    k.EnableInputWeightsType(WeightsType::F32);
    k.EnableAllInputLayout();
    k.EnableOutputLayout(DataLayout::fb);
    k.EnableBatching();
    k.EnableBiasPerFeature();
    k.EnableNonBiasTerm();
    return k;
}

JitConstants FullyConnected_fb_io_ref::GetJitConstants(const fully_connected_params& params, const DispatchData& dispatchData) const {
    JitConstants jit = Parent::GetJitConstants(params, dispatchData);

    if (!params.fused_ops.empty()) {
        auto input_dt = GetActivationType(params);
        FusedOpsConfiguration conf = { "", {"b", "f", "y", "x"}, "result", input_dt, 1 };
        jit.Merge(MakeFusedOpsJitConstants(params, { conf }));
    }
    return jit;
}

KernelsData FullyConnected_fb_io_ref::GetKernelsData(const Params& params, const optional_params& optParams) const {
    // TODO: it should be fb_io. but the original code use this kernel with yxfb and yxio
    //       (fb == fyxb flatten fyx, not yxfb flatten yxf).
    //       the order of the add operation cause some numeric changes. in order to avoid them right now we use
    //       yxfb/oiyx instead.
    // return GetCommonKernelsData(params, optParams, DataLayout::fb, WeightsLayout::io, FORCE_PRIORITY_6);
    KernelsData res = {};
    for (size_t i = 0; i < autoTuneOptions.size(); i++) {
        KernelsData kd = GetTunedKernelsDataByIndex(params,
                                                    optParams,
                                                    DataLayout::yxfb,
                                                    WeightsLayout::yxio,
                                                    static_cast<int>(i));
        if (!kd.empty()) {
            res.emplace_back(kd[0]);
        }
    }
    return res;
}

KernelsPriority FullyConnected_fb_io_ref::GetKernelsPriority(const Params& /*params*/, const optional_params& /*options*/) const {
    return FORCE_PRIORITY_6;
}
}  // namespace kernel_selector
