//===- TestPatterns.cpp - Test dialect pattern driver ---------------------===//
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

#include "TestDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
using namespace mlir;

namespace {
#include "TestPatterns.inc"
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Canonicalizer Driver.
//===----------------------------------------------------------------------===//

namespace {
struct TestPatternDriver : public FunctionPass<TestPatternDriver> {
  void runOnFunction() override {
    mlir::OwningRewritePatternList patterns;
    populateWithGenerated(&getContext(), &patterns);

    // Verify named pattern is generated with expected name.
    RewriteListBuilder<TestNamedPatternRule>::build(patterns, &getContext());

    applyPatternsGreedily(getFunction(), std::move(patterns));
  }
};
} // end anonymous namespace

static mlir::PassRegistration<TestPatternDriver>
    pass("test-patterns", "Run test dialect patterns");

//===----------------------------------------------------------------------===//
// Legalization Driver.
//===----------------------------------------------------------------------===//
namespace {
/// This pattern is a simple pattern that inlines the first region of a given
/// operation into the parent region.
struct TestRegionRewriteBlockMovement : public ConversionPattern {
  TestRegionRewriteBlockMovement(MLIRContext *ctx)
      : ConversionPattern("test.region", 1, ctx) {}

  PatternMatchResult matchAndRewrite(Operation *op, ArrayRef<Value *> operands,
                                     PatternRewriter &rewriter) const final {
    // Inline this region into the parent region.
    auto &parentRegion = *op->getContainingRegion();
    rewriter.inlineRegionBefore(op->getRegion(0), parentRegion.end());

    // Drop this operation.
    rewriter.replaceOp(op, llvm::None);
    return matchSuccess();
  }
};
/// This pattern simply erases the given operation.
struct TestDropOp : public ConversionPattern {
  TestDropOp(MLIRContext *ctx) : ConversionPattern("test.drop_op", 1, ctx) {}
  PatternMatchResult matchAndRewrite(Operation *op, ArrayRef<Value *> operands,
                                     PatternRewriter &rewriter) const final {
    rewriter.replaceOp(op, llvm::None);
    return matchSuccess();
  }
};
} // namespace

namespace {
struct TestTypeConverter : public TypeConverter {
  using TypeConverter::TypeConverter;

  LogicalResult convertType(Type t, SmallVectorImpl<Type> &results) override {
    // Drop I16 types.
    if (t.isInteger(16))
      return success();

    // Convert I64 to F64.
    if (t.isInteger(64)) {
      results.push_back(FloatType::getF64(t.getContext()));
      return success();
    }

    // Otherwise, convert the type directly.
    results.push_back(t);
    return success();
  }
};

struct TestLegalizePatternDriver
    : public ModulePass<TestLegalizePatternDriver> {
  void runOnModule() override {
    mlir::OwningRewritePatternList patterns;
    populateWithGenerated(&getContext(), &patterns);
    RewriteListBuilder<TestRegionRewriteBlockMovement, TestDropOp>::build(
        patterns, &getContext());

    TestTypeConverter converter;
    ConversionTarget target(getContext());
    target.addLegalOp<LegalOpA>();
    if (failed(applyConversionPatterns(getModule(), target, converter,
                                       std::move(patterns))))
      signalPassFailure();
  }
};
} // end anonymous namespace

static mlir::PassRegistration<TestLegalizePatternDriver>
    legalizer_pass("test-legalize-patterns",
                   "Run test dialect legalization patterns");
