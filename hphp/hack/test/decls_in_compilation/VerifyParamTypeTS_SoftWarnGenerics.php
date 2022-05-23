<?hh
// RUN: %hackc --test-compile-with-decls %s | FileCheck %s
// RUN: %hackc --test-compile-with-decls --use-serialized-decls %s | FileCheck %s

class NoGenerics {}
class WarnReifiedGenericsClass<T, Ta, <<__Warn>> reify Tb> {}
class SoftReifiedGenericsClass<T, Ta, <<__Soft>> reify Tb> {}

function test(
  NoGenerics $one,
  WarnReifiedGenericsClass<int, int, int> $t2,
  // CHECK: VerifyParamTypeTS $t2

  SoftReifiedGenericsClass<int, int, int> $t3,
  // CHECK: VerifyParamTypeTS $t3
): void {}
