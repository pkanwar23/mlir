//===- SPIRVTypes.h - MLIR SPIR-V Types -------------------------*- C++ -*-===//
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
// This file declares the types in the SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_SPIRV_SPIRVTYPES_H_
#define MLIR_SPIRV_SPIRVTYPES_H_

#include "mlir/IR/Types.h"

// Pull in all enum type definitions and utility function declarations
#include "mlir/SPIRV/SPIRVEnums.h.inc"

#include <tuple>

namespace mlir {
namespace spirv {

namespace detail {
struct ArrayTypeStorage;
struct ImageTypeStorage;
struct PointerTypeStorage;
struct RuntimeArrayTypeStorage;
} // namespace detail

namespace TypeKind {
enum Kind {
  Array = Type::FIRST_SPIRV_TYPE,
  ImageType,
  Pointer,
  RuntimeArray,
};
}

// SPIR-V array type
class ArrayType
    : public Type::TypeBase<ArrayType, Type, detail::ArrayTypeStorage> {
public:
  using Base::Base;

  static bool kindof(unsigned kind) { return kind == TypeKind::Array; }

  static ArrayType get(Type elementType, int64_t elementCount);

  Type getElementType();

  int64_t getElementCount();
};

// SPIR-V pointer type
class PointerType
    : public Type::TypeBase<PointerType, Type, detail::PointerTypeStorage> {
public:
  using Base::Base;

  static bool kindof(unsigned kind) { return kind == TypeKind::Pointer; }

  static PointerType get(Type pointeeType, StorageClass storageClass);

  Type getPointeeType();

  StorageClass getStorageClass();
  StringRef getStorageClassStr();
};

// SPIR-V run-time array type
class RuntimeArrayType
    : public Type::TypeBase<RuntimeArrayType, Type,
                            detail::RuntimeArrayTypeStorage> {
public:
  using Base::Base;

  static bool kindof(unsigned kind) { return kind == TypeKind::RuntimeArray; }

  static RuntimeArrayType get(Type elementType);

  Type getElementType();
};

// SPIR-V image type
class ImageType
    : public Type::TypeBase<ImageType, Type, detail::ImageTypeStorage> {
public:
  using Base::Base;

  static bool kindof(unsigned kind) { return kind == TypeKind::ImageType; }

  static ImageType
  get(Type elementType, Dim dim,
      ImageDepthInfo depth = ImageDepthInfo::DepthUnknown,
      ImageArrayedInfo arrayed = ImageArrayedInfo::NonArrayed,
      ImageSamplingInfo samplingInfo = ImageSamplingInfo::SingleSampled,
      ImageSamplerUseInfo samplerUse = ImageSamplerUseInfo::SamplerUnknown,
      ImageFormat format = ImageFormat::Unknown) {
    return ImageType::get(
        std::tuple<Type, Dim, ImageDepthInfo, ImageArrayedInfo,
                   ImageSamplingInfo, ImageSamplerUseInfo, ImageFormat>(
            elementType, dim, depth, arrayed, samplingInfo, samplerUse,
            format));
  }

  static ImageType
      get(std::tuple<Type, Dim, ImageDepthInfo, ImageArrayedInfo,
                     ImageSamplingInfo, ImageSamplerUseInfo, ImageFormat>);

  Type getElementType();
  Dim getDim();
  ImageDepthInfo getDepthInfo();
  ImageArrayedInfo getArrayedInfo();
  ImageSamplingInfo getSamplingInfo();
  ImageSamplerUseInfo getSamplerUseInfo();
  ImageFormat getImageFormat();
  // TODO(ravishankarm): Add support for Access qualifier
};

} // end namespace spirv
} // end namespace mlir

#endif // MLIR_SPIRV_SPIRVTYPES_H_
