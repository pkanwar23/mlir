// RUN: mlir-opt %s -split-input-file -canonicalize | FileCheck %s --dump-input=fail

// -----
// CHECK-LABEL: redundant_scast
func @redundant_scast() -> tensor<4xi8> {
  // CHECK-NEXT: constant dense<tensor<4xi8>, 10>
  // CHECK-NEXT: return
  %cst = constant dense<tensor<4xi8>, 5>
  %1 = "quant.scast"(%cst) : (tensor<4xi8>) -> tensor<4x!quant.uniform<u8:f32, 7.812500e-03:128>>
  %2 = "quant.scast"(%1) : (tensor<4x!quant.uniform<u8:f32, 7.812500e-03:128>>) -> tensor<4xi8>
  %3 = addi %2, %2 : tensor<4xi8>
  return %3 : tensor<4xi8>
}

// -----
// CHECK-LABEL: non_redundant_scast
func @non_redundant_scast() -> tensor<4x!quant.uniform<u8:f32, 7.812500e-03:128>> {
  // CHECK-NEXT: constant dense<tensor<4xi8>, 5>
  // CHECK-NEXT: scast
  // CHECK-NEXT: return
  %cst = constant dense<tensor<4xi8>, 5>
  %1 = "quant.scast"(%cst) : (tensor<4xi8>) -> tensor<4x!quant.uniform<u8:f32, 7.812500e-03:128>>
  return %1 : tensor<4x!quant.uniform<u8:f32, 7.812500e-03:128>>
}
