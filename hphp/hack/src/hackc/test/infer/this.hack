// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast %s | FileCheck %s

class ReturnType {

  // TEST-CHECK-BAL: define ReturnType.f
  // CHECK: define ReturnType.f($this: .notnull *ReturnType) : .this *HackMixed {
  // CHECK: #b0:
  // CHECK: // .column 12
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK: // .column 5
  // CHECK:   ret all n0
  // CHECK: }
  public function f(): this {
    return $this;
  }
}
