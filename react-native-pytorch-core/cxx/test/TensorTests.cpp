/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <string>

#include "TorchliveTestBase.h"

namespace {

class TorchliveTensorRuntimeTest
    : public torchlive::test::TorchliveBindingsTestBase {};

TEST_F(TorchliveTensorRuntimeTest, TensorTest) {
  std::string tensorIndexWithNumber =
      R"(
        const tensor = torch.tensor([[128], [255]]);
        const tensor1 = tensor[0];
        const tensor2 = tensor[1];
        tensor1.data[0] == 128 && tensor2.data[0] == 255;
      )";
  EXPECT_TRUE(eval(tensorIndexWithNumber).getBool());

  std::string tensorIndexWithNumberString =
      R"(
        const tensor = torch.tensor([[128], [255]]);
        const tensor1 = tensor['0'];
        const tensor2 = tensor['1'];
        tensor1.data[0] == 128 && tensor2.data[0] == 255;
      )";
  EXPECT_TRUE(eval(tensorIndexWithNumberString).getBool());

  EXPECT_TRUE(eval("torch.tensor([[128], [255]])['foo']").isUndefined());

  EXPECT_TRUE(eval("torch.tensor([[128], [255]])[-1]").isUndefined());

  EXPECT_TRUE(eval("torch.tensor([[128], [255]])[2]").isUndefined());
}

TEST_F(TorchliveTensorRuntimeTest, TensorDataTest) {
  std::string tensorWithDtypeAsUint8 =
      R"(
        const tensor = torch.tensor([0.1, 2.0, 2.7], {dtype: torch.uint8});
        tensor.dtype == torch.uint8 && tensor.data[0] == 0 && tensor.data[1] == 2 && tensor.data[2] == 2;
      )";
  EXPECT_TRUE(eval(tensorWithDtypeAsUint8).getBool());

  std::string tensorWithDtypeAsInt8 =
      R"(
        const tensor = torch.tensor([0.1, -2.0, 2.7, -2.7], {dtype: torch.int8});
        tensor.dtype == torch.int8 && tensor.data[0] == 0 && tensor.data[1] == -2 && tensor.data[2] == 2 && tensor.data[3] == -2;
      )";
  EXPECT_TRUE(eval(tensorWithDtypeAsInt8).getBool());

  std::string tensorWithDtypeAsInt16 =
      R"(
        const tensor = torch.tensor([0.1, 2.0], {dtype: torch.int16});
        const tensor2 = torch.tensor([0.1, 2.0], {dtype: torch.short});
        tensor.dtype == torch.int16 && tensor2.dtype == torch.int16;
      )";
  EXPECT_TRUE(eval(tensorWithDtypeAsInt16).getBool());

  std::string tensorWithDtypeAsInt32 =
      R"(
        const tensor = torch.tensor([0.1, 2.0], {dtype: torch.int32});
        const tensor2 = torch.tensor([0.1, 2.0], {dtype: torch.int});
        tensor.dtype == torch.int32 && tensor2.dtype == torch.int32;
      )";
  EXPECT_TRUE(eval(tensorWithDtypeAsInt32).getBool());

  std::string tensorWithDtypeAsFloat32 =
      R"(
        const tensor = torch.tensor([1.5, 2.0], {dtype: torch.float32});
        const tensor2 = torch.tensor([1.5, 2.0], {dtype: torch.float});
        tensor.dtype == torch.float32 && tensor2.dtype == torch.float32 && tensor.data[0] == 1.5 && tensor.data[1] == 2.0;
      )";
  EXPECT_TRUE(eval(tensorWithDtypeAsFloat32).getBool());

  std::string tensorWithDtypeAsFloat64 =
      R"(
        const tensor = torch.tensor([0.1, 2.0], {dtype: torch.float64});
        const tensor2 = torch.tensor([0.1, 2.0], {dtype: torch.double});
        tensor.dtype == torch.float64 && tensor2.dtype == torch.float64;
      )";
  EXPECT_TRUE(eval(tensorWithDtypeAsFloat64).getBool());

  std::string tensorWithDtypeAsInt64 =
      R"(
        const tensor = torch.tensor([128, 255], {dtype: torch.long});
        tensor.data;
      )";
  EXPECT_THROW(eval(tensorWithDtypeAsInt64), facebook::jsi::JSError);
}

TEST_F(TorchliveTensorRuntimeTest, TensorDivTest) {
  std::string tensorDivWithNumber =
      R"(
        const tensor = torch.tensor([0, 255]);
        const result = tensor.div(255);
        result.data[0] == 0 && result.data[1] == 1
      )";
  EXPECT_TRUE(eval(tensorDivWithNumber.c_str()).getBool());

  for (auto i = 0; i < 4; i++) {
    std::string tensorDivWithNumber = fmt::format(
        R"(
          const tensor = torch.arange(1, 5);
          const result = tensor.div(2);
          result.data[{}] == tensor.data[{}] / 2;
        )",
        i,
        i);
    EXPECT_TRUE(eval(tensorDivWithNumber.c_str()).getBool());
  }

  for (auto i = 0; i < 4; i++) {
    std::string tensorDivWithTensor = fmt::format(
        R"(
          const tensor1 = torch.arange(1, 5);
          const tensor2 = torch.arange(1, 5);
          const result = tensor1.div(tensor2);
          result.data[{}] == tensor1.data[{}] / tensor2.data[{}];
        )",
        i,
        i,
        i);
    EXPECT_TRUE(eval(tensorDivWithTensor.c_str()).getBool());
  }

  EXPECT_THROW(eval("torch.arange(1, 5).div()"), facebook::jsi::JSError);

  EXPECT_THROW(
      eval("torch.arange(1, 5).div(torch.arrange(3, 4), 'foo')"),
      facebook::jsi::JSError);
}

} // namespace