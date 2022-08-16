<?hh
// RUN: %hackc --test-compile-with-decls %s | FileCheck %s
// RUN: %hackc --test-compile-with-decls --use-serialized-decls %s | FileCheck %s

namespace MyTestNamespace {

  class NoGenerics {}
  class ErasedGenericsClass<T> {}
  class ReifiedGenericsClass<reify T> {}

  function test(
    NoGenerics $one,
    ErasedGenericsClass<int> $t2,
    // CHECK-NOT: VerifyParamTypeTS $t2

    ReifiedGenericsClass<int> $t3,
    // CHECK: VerifyParamTypeTS $t3

    \ReifiedGenericsClass<int> $t4,
  ): void {}

}
