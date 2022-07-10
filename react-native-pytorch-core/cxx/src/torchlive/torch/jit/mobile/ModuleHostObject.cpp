/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <torch/csrc/jit/mobile/module.h>
#include <utility>

#include "../../../common/AsyncTask.h"
#include "../../../torchlive.h"
#include "../../IValueHostObject.h"
#include "../../TensorHostObject.h"
#include "../../utils/converter.h"
#include "../../utils/helpers.h"
#include "ModuleHostObject.h"

// Namespace alias for torch to avoid namespace conflicts with torchlive::torch
namespace torch_ = torch;

namespace torchlive {
namespace torch {
namespace jit {
namespace mobile {

using namespace facebook;

using ForwardAsyncTask = common::AsyncTask<
    std::tuple<torch_::jit::mobile::Module, std::vector<torch_::jit::IValue>>,
    torch_::jit::IValue>;

ForwardAsyncTask forwardImpl(
    [](jsi::Runtime& runtime,
       const jsi::Value& thisValue,
       const jsi::Value* arguments,
       size_t count) -> ForwardAsyncTask::SetupResultType {
      if (count < 1) {
        throw jsi::JSError(runtime, "At least 1 arg is expected");
      }

      auto thiz =
          thisValue.asObject(runtime).asHostObject<ModuleHostObject>(runtime);

      auto args = thiz->mobileModule.get_method("forward")
                      .function()
                      .getSchema()
                      .arguments();

      // Two Cases in terms of number of argument required and argument provided
      // Case 1 (n_required < n_provided) we ignore the extra provided args,
      // respecting Js convention
      // Case 2 (n_required >= n_provided) we process the provided argument and
      // let libtorch check if they are enough, this would handle module with
      // default parameters
      int argCount = std::min(count, args.size() - 1);

      std::vector<torch_::jit::IValue> input;
      for (int i = 0; i < argCount; i++) {
        c10::DynamicType& dynType =
            args[i + 1].type()->expectRef<c10::DynamicType>();
        input.push_back(
            utils::converter::jsiValuetoIValue(runtime, arguments[i], dynType));
      }
      return std::make_tuple(thiz->mobileModule, input);
    },

    [](ForwardAsyncTask::SetupResultType&& input) -> torch_::jit::IValue {
      torch_::jit::mobile::Module mobileModule;
      std::vector<torch_::jit::IValue> tensors;
      std::tie(mobileModule, tensors) = input;
      c10::InferenceMode guard;
      return mobileModule.forward(std::move(tensors));
    },

    [](jsi::Runtime& runtime,
       torchlive::RuntimeExecutor,
       torch_::jit::IValue&& value) -> jsi::Value {
      return utils::converter::ivalueToJSIValue(runtime, value);
    });

ModuleHostObject::ModuleHostObject(
    jsi::Runtime& rt,
    torchlive::RuntimeExecutor rte,
    torch_::jit::mobile::Module m)
    : BaseHostObject(rt), mobileModule(m) {
  setPropertyHostFunction(rt, "forward", 1, forwardImpl.asyncPromiseFunc(rte));
  setPropertyHostFunction(rt, "forwardSync", 1, forwardImpl.syncFunc(rte));
}

} // namespace mobile
} // namespace jit
} // namespace torch
} // namespace torchlive
