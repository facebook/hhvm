// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast %s | FileCheck %s

class ReturnType {

  // TEST-CHECK-BAL: define ReturnType.f
  // CHECK: define ReturnType.f($this: *ReturnType) : .this *HackMixed {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK:   n1 = $builtins.hhbc_is_late_bound_cls(n0)
  // CHECK:   n2 = $builtins.hhbc_verify_type_pred(n0, n1)
  // CHECK:   ret n0
  // CHECK: }
  public function f(): this {
    return $this;
  }
}
