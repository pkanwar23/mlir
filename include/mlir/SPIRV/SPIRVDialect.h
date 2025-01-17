//===- SPIRVDialect.h - MLIR SPIR-V dialect ---------------------*- C++ -*-===//
//
// Copyright 2019 The MLIR Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================
//
// This file declares the SPIR-V dialect in MLIR.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_SPIRV_SPIRVDIALECT_H_
#define MLIR_SPIRV_SPIRVDIALECT_H_

#include "mlir/IR/Dialect.h"

namespace mlir {
namespace spirv {

class SPIRVDialect : public Dialect {
public:
  explicit SPIRVDialect(MLIRContext *context);

  static StringRef getDialectNamespace() { return "spv"; }

  /// Parses a type registered to this dialect.
  Type parseType(llvm::StringRef spec, Location loc) const override;

  /// Prints a type registered to this dialect.
  void printType(Type type, llvm::raw_ostream &os) const override;

private:
  /// Parses `spec` as a type and verifies it can be used in SPIR-V types.
  Type parseAndVerifyType(StringRef spec, Location loc) const;

  /// Parses `spec` as a SPIR-V array type.
  Type parseArrayType(StringRef spec, Location loc) const;

  /// Parses `spec` as a SPIR-V pointer type.
  Type parsePointerType(StringRef spec, Location loc) const;

  /// Parses `spec` as a SPIR-V run-time array type.
  Type parseRuntimeArrayType(StringRef spec, Location loc) const;

  /// Parses `spec` as a SPIR-V image type
  Type parseImageType(StringRef spec, Location loc) const;
};

} // end namespace spirv
} // end namespace mlir

#endif // MLIR_SPIRV_SPIRVDIALECT_H_
