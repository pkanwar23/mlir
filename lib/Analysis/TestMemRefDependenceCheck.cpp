//===- TestMemRefDependenceCheck.cpp - Test dep analysis ------------------===//
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
// This file implements a pass to run pair-wise memref access dependence checks.
//
//===----------------------------------------------------------------------===//

#include "mlir/Analysis/AffineAnalysis.h"
#include "mlir/Analysis/AffineStructures.h"
#include "mlir/Analysis/Passes.h"
#include "mlir/Analysis/Utils.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/StandardOps/Ops.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "test-memref-dependence-check"

using namespace mlir;

namespace {

// TODO(andydavis) Add common surrounding loop depth-wise dependence checks.
/// Checks dependences between all pairs of memref accesses in a Function.
struct TestMemRefDependenceCheck
    : public FunctionPass<TestMemRefDependenceCheck> {
  SmallVector<Operation *, 4> loadsAndStores;
  void runOnFunction() override;
};

} // end anonymous namespace

FunctionPassBase *mlir::createTestMemRefDependenceCheckPass() {
  return new TestMemRefDependenceCheck();
}

// Returns a result string which represents the direction vector (if there was
// a dependence), returns the string "false" otherwise.
static std::string
getDirectionVectorStr(bool ret, unsigned numCommonLoops, unsigned loopNestDepth,
                      ArrayRef<DependenceComponent> dependenceComponents) {
  if (!ret)
    return "false";
  if (dependenceComponents.empty() || loopNestDepth > numCommonLoops)
    return "true";
  std::string result;
  for (unsigned i = 0, e = dependenceComponents.size(); i < e; ++i) {
    std::string lbStr = "-inf";
    if (dependenceComponents[i].lb.hasValue() &&
        dependenceComponents[i].lb.getValue() !=
            std::numeric_limits<int64_t>::min())
      lbStr = std::to_string(dependenceComponents[i].lb.getValue());

    std::string ubStr = "+inf";
    if (dependenceComponents[i].ub.hasValue() &&
        dependenceComponents[i].ub.getValue() !=
            std::numeric_limits<int64_t>::max())
      ubStr = std::to_string(dependenceComponents[i].ub.getValue());

    result += "[" + lbStr + ", " + ubStr + "]";
  }
  return result;
}

// For each access in 'loadsAndStores', runs a depence check between this
// "source" access and all subsequent "destination" accesses in
// 'loadsAndStores'. Emits the result of the dependence check as a note with
// the source access.
static void checkDependences(ArrayRef<Operation *> loadsAndStores) {
  for (unsigned i = 0, e = loadsAndStores.size(); i < e; ++i) {
    auto *srcOpInst = loadsAndStores[i];
    MemRefAccess srcAccess(srcOpInst);
    for (unsigned j = 0; j < e; ++j) {
      auto *dstOpInst = loadsAndStores[j];
      MemRefAccess dstAccess(dstOpInst);

      unsigned numCommonLoops =
          getNumCommonSurroundingLoops(*srcOpInst, *dstOpInst);
      for (unsigned d = 1; d <= numCommonLoops + 1; ++d) {
        FlatAffineConstraints dependenceConstraints;
        llvm::SmallVector<DependenceComponent, 2> dependenceComponents;
        DependenceResult result = checkMemrefAccessDependence(
            srcAccess, dstAccess, d, &dependenceConstraints,
            &dependenceComponents);
        assert(result.value != DependenceResult::Failure);
        bool ret = hasDependence(result);
        // TODO(andydavis) Print dependence type (i.e. RAW, etc) and print
        // distance vectors as: ([2, 3], [0, 10]). Also, shorten distance
        // vectors from ([1, 1], [3, 3]) to (1, 3).
        srcOpInst->emitRemark("dependence from ")
            << i << " to " << j << " at depth " << d << " = "
            << getDirectionVectorStr(ret, numCommonLoops, d,
                                     dependenceComponents);
      }
    }
  }
}

// Walks the Function 'f' adding load and store ops to 'loadsAndStores'.
// Runs pair-wise dependence checks.
void TestMemRefDependenceCheck::runOnFunction() {
  // Collect the loads and stores within the function.
  loadsAndStores.clear();
  getFunction().walk([&](Operation *op) {
    if (isa<LoadOp>(op) || isa<StoreOp>(op))
      loadsAndStores.push_back(op);
  });

  checkDependences(loadsAndStores);
}

static PassRegistration<TestMemRefDependenceCheck>
    pass("test-memref-dependence-check",
         "Checks dependences between all pairs of memref accesses.");
