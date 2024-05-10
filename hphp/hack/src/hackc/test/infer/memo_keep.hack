// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast --keep-memo %s | FileCheck %s

class C {
  // TEST-CHECK-BAL: define C.memometh_inst
  // CHECK: define C.memometh_inst($this: .notnull *C, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C_2ememometh__inst: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &gconst::HH::MEMOIZE_IC_TYPE_INACCESSIBLE
  // CHECK:   n1: *HackMixed = load &$this
  // CHECK:   n2 = $builtins.hhbc_check_this(n1)
  // CHECK:   n3: *HackMixed = load &$a
  // CHECK:   n4 = $builtins.hhbc_get_memo_key_l(n3)
  // CHECK:   store &$0 <- n4: *HackMixed
  // CHECK:   n5: *HackMixed = load &$b
  // CHECK:   n6 = $builtins.hhbc_get_memo_key_l(n5)
  // CHECK:   store &$1 <- n6: *HackMixed
  // CHECK:   n7: *HackMixed = load &$this
  // CHECK:   n8: *HackMixed = load &$0
  // CHECK:   n9: *HackMixed = load &$1
  // CHECK:   n10 = $builtins.hack_memo_isset(&memocache::_C_2ememometh__inst, n7, n8, n9)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n10)
  // CHECK:   n11 = $builtins.hack_memo_get(&memocache::_C_2ememometh__inst, n7, n8, n9)
  // CHECK:   ret n11
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n10)
  // CHECK:   n12: *C = load &$this
  // CHECK:   n13: *HackMixed = load &$a
  // CHECK:   n14: *HackMixed = load &$b
  // CHECK:   n15 = $builtins.hhbc_create_special_implicit_context(n0, null)
  // CHECK:   n16 = $builtins.hhbc_set_implicit_context_by_value(n15)
  // CHECK:   store &$2 <- n16: *HackMixed
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   n17 = n12.?.memometh_inst$memoize_impl(n13, n14)
  // CHECK:   jmp b5
  // CHECK:   .handlers b4
  // CHECK: #b4(n18: *HackMixed):
  // CHECK:   n19: *HackMixed = load &$2
  // CHECK:   n20 = $builtins.hhbc_set_implicit_context_by_value(n19)
  // CHECK:   throw n18
  // CHECK: #b5:
  // CHECK:   n21: *HackMixed = load &$2
  // CHECK:   n22 = $builtins.hhbc_set_implicit_context_by_value(n21)
  // CHECK:   n23: *HackMixed = load &$this
  // CHECK:   n24: *HackMixed = load &$0
  // CHECK:   n25: *HackMixed = load &$1
  // CHECK:   n26 = $builtins.hhbc_memo_set(&memocache::_C_2ememometh__inst, n23, n24, n25, n17)
  // CHECK:   ret n26
  // CHECK: }
  <<__Memoize>>
  public function memometh_inst(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-BAL: define C$static.memometh_static
  // CHECK: define C$static.memometh_static($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C$static_2ememometh__static: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &gconst::HH::MEMOIZE_IC_TYPE_INACCESSIBLE
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK:   n2 = $builtins.hhbc_get_memo_key_l(n1)
  // CHECK:   store &$0 <- n2: *HackMixed
  // CHECK:   n3: *HackMixed = load &$b
  // CHECK:   n4 = $builtins.hhbc_get_memo_key_l(n3)
  // CHECK:   store &$1 <- n4: *HackMixed
  // CHECK:   n5: *HackMixed = load &$this
  // CHECK:   n6: *HackMixed = load &$0
  // CHECK:   n7: *HackMixed = load &$1
  // CHECK:   n8 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__static, n5, n6, n7)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n8)
  // CHECK:   n9 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__static, n5, n6, n7)
  // CHECK:   ret n9
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n8)
  // CHECK:   n10: *HackMixed = load &$a
  // CHECK:   n11: *HackMixed = load &$b
  // CHECK:   n12 = $builtins.hhbc_create_special_implicit_context(n0, null)
  // CHECK:   n13 = $builtins.hhbc_set_implicit_context_by_value(n12)
  // CHECK:   store &$2 <- n13: *HackMixed
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   n14: *C$static = load &$this
  // CHECK:   n15 = C$static.memometh_static$memoize_impl(n14, n10, n11)
  // CHECK:   jmp b5
  // CHECK:   .handlers b4
  // CHECK: #b4(n16: *HackMixed):
  // CHECK:   n17: *HackMixed = load &$2
  // CHECK:   n18 = $builtins.hhbc_set_implicit_context_by_value(n17)
  // CHECK:   throw n16
  // CHECK: #b5:
  // CHECK:   n19: *HackMixed = load &$2
  // CHECK:   n20 = $builtins.hhbc_set_implicit_context_by_value(n19)
  // CHECK:   n21: *HackMixed = load &$this
  // CHECK:   n22: *HackMixed = load &$0
  // CHECK:   n23: *HackMixed = load &$1
  // CHECK:   n24 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__static, n21, n22, n23, n15)
  // CHECK:   ret n24
  // CHECK: }
  <<__Memoize>>
  public static function memometh_static(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-1: define C$static.memometh_static$memoize_impl
  // CHECK: define C$static.memometh_static$memoize_impl($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {

  // TEST-CHECK-BAL: define C$static.memometh_lsb
  // CHECK: define C$static.memometh_lsb($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C$static_2ememometh__lsb: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &gconst::HH::MEMOIZE_IC_TYPE_INACCESSIBLE
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK:   n2 = $builtins.hhbc_get_memo_key_l(n1)
  // CHECK:   store &$0 <- n2: *HackMixed
  // CHECK:   n3: *HackMixed = load &$b
  // CHECK:   n4 = $builtins.hhbc_get_memo_key_l(n3)
  // CHECK:   store &$1 <- n4: *HackMixed
  // CHECK:   n5: *HackMixed = load &$this
  // CHECK:   n6: *HackMixed = load &$0
  // CHECK:   n7: *HackMixed = load &$1
  // CHECK:   n8 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__lsb, n5, n6, n7)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n8)
  // CHECK:   n9 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__lsb, n5, n6, n7)
  // CHECK:   ret n9
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n8)
  // CHECK:   n10: *HackMixed = load &$a
  // CHECK:   n11: *HackMixed = load &$b
  // CHECK:   n12 = $builtins.hhbc_create_special_implicit_context(n0, null)
  // CHECK:   n13 = $builtins.hhbc_set_implicit_context_by_value(n12)
  // CHECK:   store &$2 <- n13: *HackMixed
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   n14: *C$static = load &$this
  // CHECK:   n15 = C$static.memometh_lsb$memoize_impl(n14, n10, n11)
  // CHECK:   jmp b5
  // CHECK:   .handlers b4
  // CHECK: #b4(n16: *HackMixed):
  // CHECK:   n17: *HackMixed = load &$2
  // CHECK:   n18 = $builtins.hhbc_set_implicit_context_by_value(n17)
  // CHECK:   throw n16
  // CHECK: #b5:
  // CHECK:   n19: *HackMixed = load &$2
  // CHECK:   n20 = $builtins.hhbc_set_implicit_context_by_value(n19)
  // CHECK:   n21: *HackMixed = load &$this
  // CHECK:   n22: *HackMixed = load &$0
  // CHECK:   n23: *HackMixed = load &$1
  // CHECK:   n24 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__lsb, n21, n22, n23, n15)
  // CHECK:   ret n24
  // CHECK: }
  <<__MemoizeLSB>>
  public static function memometh_lsb(int $a, int $b)[]: int {
    return $a + $b;
  }
}

// TEST-CHECK-1: define C$static.memometh_lsb$memoize_impl
// CHECK: define C$static.memometh_lsb$memoize_impl($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {

// TEST-CHECK-BAL: define $root.memofunc
// CHECK: define $root.memofunc($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
// CHECK: local memocache::_$root_2ememofunc: *void, $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &gconst::HH::MEMOIZE_IC_TYPE_INACCESSIBLE
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_get_memo_key_l(n1)
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK:   n3: *HackMixed = load &$b
// CHECK:   n4 = $builtins.hhbc_get_memo_key_l(n3)
// CHECK:   store &$1 <- n4: *HackMixed
// CHECK:   n5: *HackMixed = load &$0
// CHECK:   n6: *HackMixed = load &$1
// CHECK:   n7 = $builtins.hack_memo_isset(&memocache::_$root_2ememofunc, n5, n6)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n8 = $builtins.hack_memo_get(&memocache::_$root_2ememofunc, n5, n6)
// CHECK:   ret n8
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   n9: *HackMixed = load &$a
// CHECK:   n10: *HackMixed = load &$b
// CHECK:   n11 = $builtins.hhbc_create_special_implicit_context(n0, null)
// CHECK:   n12 = $builtins.hhbc_set_implicit_context_by_value(n11)
// CHECK:   store &$2 <- n12: *HackMixed
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   n13 = $root.memofunc$memoize_impl(null, n9, n10)
// CHECK:   jmp b5
// CHECK:   .handlers b4
// CHECK: #b4(n14: *HackMixed):
// CHECK:   n15: *HackMixed = load &$2
// CHECK:   n16 = $builtins.hhbc_set_implicit_context_by_value(n15)
// CHECK:   throw n14
// CHECK: #b5:
// CHECK:   n17: *HackMixed = load &$2
// CHECK:   n18 = $builtins.hhbc_set_implicit_context_by_value(n17)
// CHECK:   n19: *HackMixed = load &$0
// CHECK:   n20: *HackMixed = load &$1
// CHECK:   n21 = $builtins.hhbc_memo_set(&memocache::_$root_2ememofunc, n19, n20, n13)
// CHECK:   ret n21
// CHECK: }

<<__Memoize>>
function memofunc(int $a, int $b)[]: int {
  return $a + $b;
}

// TEST-CHECK-1: define $root.memofunc$memoize_impl
// CHECK: define $root.memofunc$memoize_impl($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {

// TEST-CHECK-BAL: define .async $root.memo_async_func
// CHECK: define .async $root.memo_async_func($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : .awaitable .notnull *HackInt {
// CHECK: local memocache::_$root_2ememo__async__func: *void, $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &gconst::HH::MEMOIZE_IC_TYPE_INACCESSIBLE
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_get_memo_key_l(n1)
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK:   n3: *HackMixed = load &$b
// CHECK:   n4 = $builtins.hhbc_get_memo_key_l(n3)
// CHECK:   store &$1 <- n4: *HackMixed
// CHECK:   n5: *HackMixed = load &$0
// CHECK:   n6: *HackMixed = load &$1
// CHECK:   n7 = $builtins.hack_memo_isset(&memocache::_$root_2ememo__async__func, n5, n6)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n8 = $builtins.hack_memo_get(&memocache::_$root_2ememo__async__func, n5, n6)
// CHECK:   ret n8
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   n9: *HackMixed = load &$a
// CHECK:   n10: *HackMixed = load &$b
// CHECK:   n11 = $builtins.hhbc_create_special_implicit_context(n0, null)
// CHECK:   n12 = $builtins.hhbc_set_implicit_context_by_value(n11)
// CHECK:   store &$2 <- n12: *HackMixed
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   n13 = $root.memo_async_func$memoize_impl(null, n9, n10)
// CHECK:   n14 = $builtins.hhbc_await(n13)
// CHECK:   jmp b5
// CHECK:   .handlers b4
// CHECK: #b4(n15: *HackMixed):
// CHECK:   n16: *HackMixed = load &$2
// CHECK:   n17 = $builtins.hhbc_set_implicit_context_by_value(n16)
// CHECK:   throw n15
// CHECK: #b5:
// CHECK:   n18: *HackMixed = load &$0
// CHECK:   n19: *HackMixed = load &$1
// CHECK:   n20 = $builtins.hhbc_memo_set(&memocache::_$root_2ememo__async__func, n18, n19, n14)
// CHECK:   n21: *HackMixed = load &$2
// CHECK:   n22 = $builtins.hhbc_set_implicit_context_by_value(n21)
// CHECK:   ret n20
// CHECK: }

<<__Memoize>>
async function memo_async_func(int $a, int $b)[]: Awaitable<int> {
  return $a + $b;
}

// TEST-CHECK-1: define .async $root.memo_async_func$memoize_impl
// CHECK: define .async $root.memo_async_func$memoize_impl($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : .awaitable .notnull *HackInt {


// TEST-CHECK-BAL: define $root.memo_with_pure_zero_param($this: *void) : .notnull *HackInt {
// CHECK: define $root.memo_with_pure_zero_param($this: *void) : .notnull *HackInt {
// CHECK: local memocache::_$root_2ememo__with__pure__zero__param: *void, $0: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &gconst::HH::MEMOIZE_IC_TYPE_INACCESSIBLE
// CHECK:   n1 = $builtins.hack_memo_isset(&memocache::_$root_2ememo__with__pure__zero__param)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK:   n2 = $builtins.hack_memo_get(&memocache::_$root_2ememo__with__pure__zero__param)
// CHECK:   ret n2
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK:   n3 = $builtins.hhbc_create_special_implicit_context(n0, null)
// CHECK:   n4 = $builtins.hhbc_set_implicit_context_by_value(n3)
// CHECK:   store &$0 <- n4: *HackMixed
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   n5 = $root.memo_with_pure_zero_param$memoize_impl(null)
// CHECK:   jmp b5
// CHECK:   .handlers b4
// CHECK: #b4(n6: *HackMixed):
// CHECK:   n7: *HackMixed = load &$0
// CHECK:   n8 = $builtins.hhbc_set_implicit_context_by_value(n7)
// CHECK:   throw n6
// CHECK: #b5:
// CHECK:   n9: *HackMixed = load &$0
// CHECK:   n10 = $builtins.hhbc_set_implicit_context_by_value(n9)
// CHECK:   n11 = $builtins.hhbc_memo_set(&memocache::_$root_2ememo__with__pure__zero__param, n5)
// CHECK:   ret n11
// CHECK: }


<<__Memoize>>
function memo_with_pure_zero_param()[]: int {
  return 42;
}
