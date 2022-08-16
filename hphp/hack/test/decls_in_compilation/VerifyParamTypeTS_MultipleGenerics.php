<?hh
// RUN: %hackc --test-compile-with-decls %s | FileCheck %s
// RUN: %hackc --test-compile-with-decls --use-serialized-decls %s | FileCheck %s

class NoGenerics {}
class ErasedGenericsClass<T, Ta, Tb> {}
class ReifiedGenericsClass<T, Ta, reify Tb> {}

function test(
  NoGenerics $one,
  ErasedGenericsClass<int, int, int> $t2,
  // CHECK-NOT: VerifyParamTypeTS $t2

  ReifiedGenericsClass<int, int, int> $t3,
  // CHECK: VerifyParamTypeTS $t3
): void {}
