// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

class C {
  public function __construct(private dict<int, mixed> $dict) {}

  // TEST-CHECK-BAL: define C.test1
  // CHECK: define C.test1($this: *C, $idx: *HackInt) : *HackMixed {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK:   n1 = $builtins.hhbc_check_this(n0)
  // CHECK:   n2: *HackMixed = load &$idx
  // CHECK:   n3: *HackMixed = load n0.?.dict
  // CHECK:   n4 = $builtins.hack_array_cow_unset(n3, n2)
  // CHECK:   store n0.?.dict <- n4: *HackMixed
  // CHECK:   n5 = $builtins.hhbc_is_late_bound_cls(n0)
  // CHECK:   n6 = $builtins.hhbc_verify_type_pred(n0, n5)
  // CHECK:   ret n0
  // CHECK: }
  public function test1(int $idx): this {
    unset($this->dict[$idx]);
    return $this;
  }
}

// TEST-CHECK-BAL: define $root.test2
// CHECK: define $root.test2($this: *void, $dict: *HackDict, $idx: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(9))))
// CHECK:   n1: *HackMixed = load &$dict
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3: *HackMixed = load &$idx
// CHECK:   n4 = $builtins.hack_array_cow_unset(n1, n3)
// CHECK:   store &$dict <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }
function test2(dict<int, mixed> $dict, int $idx) : void  {
  unset($dict[$idx]);
}
