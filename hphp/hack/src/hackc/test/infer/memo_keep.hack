// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast --keep-memo %s | FileCheck %s

class C {
  // TEST-CHECK-BAL: define C.memometh_inst
  // CHECK: define C.memometh_inst($this: .notnull *C, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C_2ememometh__inst: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK:   n1 = $builtins.hhbc_check_this(n0)
  // CHECK:   n2: *HackMixed = load &$a
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK:   store &$0 <- n3: *HackMixed
  // CHECK:   n4: *HackMixed = load &$b
  // CHECK:   n5 = $builtins.hhbc_get_memo_key_l(n4)
  // CHECK:   store &$1 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$this
  // CHECK:   n7: *HackMixed = load &$0
  // CHECK:   n8: *HackMixed = load &$1
  // CHECK:   n9 = $builtins.hack_memo_isset(&memocache::_C_2ememometh__inst, n6, n7, n8)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n9)
  // CHECK:   n10 = $builtins.hack_memo_get(&memocache::_C_2ememometh__inst, n6, n7, n8)
  // CHECK:   ret n10
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n9)
  // CHECK:   n11: *C = load &$this
  // CHECK:   n12: *HackMixed = load &$a
  // CHECK:   n13: *HackMixed = load &$b
  // CHECK:   n14 = $builtins.hhbc_get_inaccessible_implicit_context()
  // CHECK:   n15 = $builtins.hhbc_set_implicit_context_by_value(n14)
  // CHECK:   store &$2 <- n15: *HackMixed
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   n16 = n11.?.memometh_inst$memoize_impl(n12, n13)
  // CHECK:   jmp b5
  // CHECK:   .handlers b4
  // CHECK: #b4(n17: *HackMixed):
  // CHECK:   n18: *HackMixed = load &$2
  // CHECK:   n19 = $builtins.hhbc_set_implicit_context_by_value(n18)
  // CHECK:   throw n17
  // CHECK: #b5:
  // CHECK:   n20: *HackMixed = load &$2
  // CHECK:   n21 = $builtins.hhbc_set_implicit_context_by_value(n20)
  // CHECK:   n22: *HackMixed = load &$this
  // CHECK:   n23: *HackMixed = load &$0
  // CHECK:   n24: *HackMixed = load &$1
  // CHECK:   n25 = $builtins.hhbc_memo_set(&memocache::_C_2ememometh__inst, n22, n23, n24, n16)
  // CHECK:   ret n25
  // CHECK: }
  <<__Memoize>>
  public function memometh_inst(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-BAL: define C$static.memometh_static
  // CHECK: define C$static.memometh_static($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C$static_2ememometh__static: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2: *HackMixed = load &$b
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK:   store &$1 <- n3: *HackMixed
  // CHECK:   n4: *HackMixed = load &$this
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6: *HackMixed = load &$1
  // CHECK:   n7 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__static, n4, n5, n6)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   n8 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__static, n4, n5, n6)
  // CHECK:   ret n8
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n9: *HackMixed = load &$a
  // CHECK:   n10: *HackMixed = load &$b
  // CHECK:   n11 = $builtins.hhbc_get_inaccessible_implicit_context()
  // CHECK:   n12 = $builtins.hhbc_set_implicit_context_by_value(n11)
  // CHECK:   store &$2 <- n12: *HackMixed
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   n13: *C$static = load &$this
  // CHECK:   n14 = C$static.memometh_static$memoize_impl(n13, n9, n10)
  // CHECK:   jmp b5
  // CHECK:   .handlers b4
  // CHECK: #b4(n15: *HackMixed):
  // CHECK:   n16: *HackMixed = load &$2
  // CHECK:   n17 = $builtins.hhbc_set_implicit_context_by_value(n16)
  // CHECK:   throw n15
  // CHECK: #b5:
  // CHECK:   n18: *HackMixed = load &$2
  // CHECK:   n19 = $builtins.hhbc_set_implicit_context_by_value(n18)
  // CHECK:   n20: *HackMixed = load &$this
  // CHECK:   n21: *HackMixed = load &$0
  // CHECK:   n22: *HackMixed = load &$1
  // CHECK:   n23 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__static, n20, n21, n22, n14)
  // CHECK:   ret n23
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
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2: *HackMixed = load &$b
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK:   store &$1 <- n3: *HackMixed
  // CHECK:   n4: *HackMixed = load &$this
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6: *HackMixed = load &$1
  // CHECK:   n7 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__lsb, n4, n5, n6)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   n8 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__lsb, n4, n5, n6)
  // CHECK:   ret n8
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n9: *HackMixed = load &$a
  // CHECK:   n10: *HackMixed = load &$b
  // CHECK:   n11 = $builtins.hhbc_get_inaccessible_implicit_context()
  // CHECK:   n12 = $builtins.hhbc_set_implicit_context_by_value(n11)
  // CHECK:   store &$2 <- n12: *HackMixed
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   n13: *C$static = load &$this
  // CHECK:   n14 = C$static.memometh_lsb$memoize_impl(n13, n9, n10)
  // CHECK:   jmp b5
  // CHECK:   .handlers b4
  // CHECK: #b4(n15: *HackMixed):
  // CHECK:   n16: *HackMixed = load &$2
  // CHECK:   n17 = $builtins.hhbc_set_implicit_context_by_value(n16)
  // CHECK:   throw n15
  // CHECK: #b5:
  // CHECK:   n18: *HackMixed = load &$2
  // CHECK:   n19 = $builtins.hhbc_set_implicit_context_by_value(n18)
  // CHECK:   n20: *HackMixed = load &$this
  // CHECK:   n21: *HackMixed = load &$0
  // CHECK:   n22: *HackMixed = load &$1
  // CHECK:   n23 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__lsb, n20, n21, n22, n14)
  // CHECK:   ret n23
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
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
// CHECK:   store &$1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5: *HackMixed = load &$1
// CHECK:   n6 = $builtins.hack_memo_isset(&memocache::_$root_2ememofunc, n4, n5)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK:   n7 = $builtins.hack_memo_get(&memocache::_$root_2ememofunc, n4, n5)
// CHECK:   ret n7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK:   n8: *HackMixed = load &$a
// CHECK:   n9: *HackMixed = load &$b
// CHECK:   n10 = $builtins.hhbc_get_inaccessible_implicit_context()
// CHECK:   n11 = $builtins.hhbc_set_implicit_context_by_value(n10)
// CHECK:   store &$2 <- n11: *HackMixed
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   n12 = $root.memofunc$memoize_impl(null, n8, n9)
// CHECK:   jmp b5
// CHECK:   .handlers b4
// CHECK: #b4(n13: *HackMixed):
// CHECK:   n14: *HackMixed = load &$2
// CHECK:   n15 = $builtins.hhbc_set_implicit_context_by_value(n14)
// CHECK:   throw n13
// CHECK: #b5:
// CHECK:   n16: *HackMixed = load &$2
// CHECK:   n17 = $builtins.hhbc_set_implicit_context_by_value(n16)
// CHECK:   n18: *HackMixed = load &$0
// CHECK:   n19: *HackMixed = load &$1
// CHECK:   n20 = $builtins.hhbc_memo_set(&memocache::_$root_2ememofunc, n18, n19, n12)
// CHECK:   ret n20
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
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
// CHECK:   store &$1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5: *HackMixed = load &$1
// CHECK:   n6 = $builtins.hack_memo_isset(&memocache::_$root_2ememo__async__func, n4, n5)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK:   n7 = $builtins.hack_memo_get(&memocache::_$root_2ememo__async__func, n4, n5)
// CHECK:   ret n7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK:   n8: *HackMixed = load &$a
// CHECK:   n9: *HackMixed = load &$b
// CHECK:   n10 = $builtins.hhbc_get_inaccessible_implicit_context()
// CHECK:   n11 = $builtins.hhbc_set_implicit_context_by_value(n10)
// CHECK:   store &$2 <- n11: *HackMixed
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   n12 = $root.memo_async_func$memoize_impl(null, n8, n9)
// CHECK:   n13 = $builtins.hhbc_await(n12)
// CHECK:   jmp b5
// CHECK:   .handlers b4
// CHECK: #b4(n14: *HackMixed):
// CHECK:   n15: *HackMixed = load &$2
// CHECK:   n16 = $builtins.hhbc_set_implicit_context_by_value(n15)
// CHECK:   throw n14
// CHECK: #b5:
// CHECK:   n17: *HackMixed = load &$0
// CHECK:   n18: *HackMixed = load &$1
// CHECK:   n19 = $builtins.hhbc_memo_set(&memocache::_$root_2ememo__async__func, n17, n18, n13)
// CHECK:   n20: *HackMixed = load &$2
// CHECK:   n21 = $builtins.hhbc_set_implicit_context_by_value(n20)
// CHECK:   ret n19
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
// CHECK:   n0 = $builtins.hack_memo_isset(&memocache::_$root_2ememo__with__pure__zero__param)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n0)
// CHECK:   n1 = $builtins.hack_memo_get(&memocache::_$root_2ememo__with__pure__zero__param)
// CHECK:   ret n1
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n0)
// CHECK:   n2 = $builtins.hhbc_get_inaccessible_implicit_context()
// CHECK:   n3 = $builtins.hhbc_set_implicit_context_by_value(n2)
// CHECK:   store &$0 <- n3: *HackMixed
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   n4 = $root.memo_with_pure_zero_param$memoize_impl(null)
// CHECK:   jmp b5
// CHECK:   .handlers b4
// CHECK: #b4(n5: *HackMixed):
// CHECK:   n6: *HackMixed = load &$0
// CHECK:   n7 = $builtins.hhbc_set_implicit_context_by_value(n6)
// CHECK:   throw n5
// CHECK: #b5:
// CHECK:   n8: *HackMixed = load &$0
// CHECK:   n9 = $builtins.hhbc_set_implicit_context_by_value(n8)
// CHECK:   n10 = $builtins.hhbc_memo_set(&memocache::_$root_2ememo__with__pure__zero__param, n4)
// CHECK:   ret n10
// CHECK: }


<<__Memoize>>
function memo_with_pure_zero_param()[]: int {
  return 42;
}
