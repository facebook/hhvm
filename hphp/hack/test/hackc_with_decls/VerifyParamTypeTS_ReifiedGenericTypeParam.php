<?hh
// RUN: %hackc --test-compile-with-decls %s | FileCheck %s
// RUN: %hackc --test-compile-with-decls --use-serialized-decls %s | FileCheck %s

// Should not look up T
function foo<reify T>(T $_a): void {}
// CHECK: VerifyParamTypeTS $_a

class Bar<reify T>{
  // Should not look up T
  public function quux(T $_b): void {}
  // CHECK: VerifyParamTypeTS $_b

  public function z(Bar<int> $_c): void {}
  // CHECK: VerifyParamTypeTS $_c
}
