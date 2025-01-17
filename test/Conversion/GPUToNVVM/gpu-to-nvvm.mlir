// RUN: mlir-opt %s -lower-gpu-ops-to-nvvm-ops | FileCheck %s

// CHECK-LABEL: func @gpu_index_ops()
func @gpu_index_ops()
    attributes { gpu.kernel } {
  // CHECK: %0 = nvvm.read.ptx.sreg.tid.x : !llvm.i32
  %tIdX = "gpu.thread_id"() {dimension: "x"} : () -> (index)
  // CHECK: %1 = nvvm.read.ptx.sreg.tid.y : !llvm.i32
  %tIdY = "gpu.thread_id"() {dimension: "y"} : () -> (index)
  // CHECK: %2 = nvvm.read.ptx.sreg.tid.z : !llvm.i32
  %tIdZ = "gpu.thread_id"() {dimension: "z"} : () -> (index)

  // CHECK: %3 = nvvm.read.ptx.sreg.ntid.x : !llvm.i32
  %bDimX = "gpu.block_dim"() {dimension: "x"} : () -> (index)
  // CHECK: %4 = nvvm.read.ptx.sreg.ntid.y : !llvm.i32
  %bDimY = "gpu.block_dim"() {dimension: "y"} : () -> (index)
  // CHECK: %5 = nvvm.read.ptx.sreg.ntid.z : !llvm.i32
  %bDimZ = "gpu.block_dim"() {dimension: "z"} : () -> (index)

  // CHECK: %6 = nvvm.read.ptx.sreg.ctaid.x : !llvm.i32
  %bIdX = "gpu.block_id"() {dimension: "x"} : () -> (index)
  // CHECK: %7 = nvvm.read.ptx.sreg.ctaid.y : !llvm.i32
  %bIdY = "gpu.block_id"() {dimension: "y"} : () -> (index)
  // CHECK: %8 = nvvm.read.ptx.sreg.ctaid.z : !llvm.i32
  %bIdZ = "gpu.block_id"() {dimension: "z"} : () -> (index)

  // CHECK: %9 = nvvm.read.ptx.sreg.nctaid.x : !llvm.i32
  %gDimX = "gpu.grid_dim"() {dimension: "x"} : () -> (index)
  // CHECK: %10 = nvvm.read.ptx.sreg.nctaid.y : !llvm.i32
  %gDimY = "gpu.grid_dim"() {dimension: "y"} : () -> (index)
  // CHECK: %11 = nvvm.read.ptx.sreg.nctaid.z : !llvm.i32
  %gDimZ = "gpu.grid_dim"() {dimension: "z"} : () -> (index)

  std.return
}
