// ======================================================================== //
// Copyright 2019 Intel Corporation                                         //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "common.h"
#include "volume.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
struct ValueSelector : public ManagedObject
{
};
#else
typedef ManagedObject ValueSelector;
#endif

typedef ValueSelector *VKLValueSelector;

OPENVKL_INTERFACE VKLValueSelector vklNewValueSelector(VKLVolume volume);

OPENVKL_INTERFACE
void vklValueSelectorSetRanges(VKLValueSelector valueSelector,
                               size_t numRanges,
                               const vkl_range1f *ranges);

OPENVKL_INTERFACE
void vklValueSelectorSetValues(VKLValueSelector valueSelector,
                               size_t numValues,
                               const float *values);

#ifdef __cplusplus
}  // extern "C"
#endif
