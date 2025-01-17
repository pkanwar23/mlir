//===- GPUToCUDAPass.h - MLIR CUDA runtime support --------------*- C++ -*-===//
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
#ifndef MLIR_CONVERSION_GPUTOCUDA_GPUTOCUDAPASS_H_
#define MLIR_CONVERSION_GPUTOCUDA_GPUTOCUDAPASS_H_

namespace mlir {

class ModulePassBase;

/// Creates a pass to convert kernel functions into CUBIN blobs.
///
/// This transformation takes the body of each function that is annotated with
/// the 'nvvm.kernel' attribute, copies it to a new LLVM module, compiles the
/// module with help of the nvptx backend and the CUDA driver into a CUDA
/// binary blob (cubin) and attaches such blob as a string attribute named
/// 'nvvm.cubin' to the kernel function.
/// After the transformation, the body of the kernel function is removed (i.e.,
/// it is turned into a declaration).
ModulePassBase *createConvertGPUKernelToCubinPass();

/// Creates a pass to convert a gpu.launch_func operation into a sequence of
/// CUDA calls.
///
/// This pass does not generate code to call CUDA directly but instead uses a
/// small wrapper library that exports a stable and conveniently typed ABI
/// ontop of CUDA.
ModulePassBase *createConvertGpuLaunchFuncToCudaCallsPass();

} // namespace mlir

#endif // MLIR_CONVERSION_GPUTOCUDA_GPUTOCUDAPASS_H_
