// ======================================================================== //
// Copyright 2018 Intel Corporation                                         //
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

#include "VLYCommon.h"
#include "objectFactory.h"
#include "ospcommon/memory/RefCount.h"
#include "ospcommon/utility/ParameterizedObject.h"

namespace volley {

  struct VOLLEY_SDK_INTERFACE ManagedObject
      : public ospcommon::memory::RefCount,
        public ospcommon::utility::ParameterizedObject
  {
    using VLY_PTR = ManagedObject *;

    ManagedObject() = default;

    virtual ~ManagedObject() override;

    // commit the object's outstanding changes (such as changed parameters)
    virtual void commit() {}

    // common function to help printf-debugging; every derived class should
    // overrride this!
    virtual std::string toString() const;

    // subtype of this ManagedObject
    VLYDataType managedObjectType{VLY_UNKNOWN};
  };

  template <typename VOLLEY_CLASS, VLYDataType VLY_TYPE>
  inline VOLLEY_CLASS *createInstanceHelper(const std::string &type)
  {
    static_assert(std::is_base_of<ManagedObject, VOLLEY_CLASS>::value,
                  "createInstanceHelper<>() is only for Volley classes, not"
                  " generic types!");

    auto *object = objectFactory<VOLLEY_CLASS, VLY_TYPE>(type);

    // denote the subclass type in the ManagedObject base class.
    if (object) {
      object->managedObjectType = VLY_TYPE;
    }

    return object;
  }
}  // namespace volley
