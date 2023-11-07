// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast --keep-memo %s | FileCheck %s

class C {
  // TEST-CHECK-BAL: define C.memometh_inst
  // CHECK: define C.memometh_inst($this: *C, $a: *HackInt, $b: *HackInt) : *HackInt {
  // CHECK: local memocache::_C_2ememometh__inst: *void, $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK:   n1 = $builtins.hhbc_check_this(n0)
  // CHECK:   n2: *HackMixed = load &$a
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK:   store &$0 <- n3: *HackMixed
  // CHECK:   n4: *HackMixed = load &$b
  // CHECK:   n5 = $builtins.hhbc_get_memo_key_l(n4)
  // CHECK:   store &$1 <- n5: *HackMixed
  // CHECK:   n6 = $builtins.hack_memo_isset(&memocache::_C_2ememometh__inst, n0, n3, n5)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n6)
  // CHECK:   n7 = $builtins.hack_memo_get(&memocache::_C_2ememometh__inst, n0, n3, n5)
  // CHECK:   ret n7
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n6)
  // CHECK:   n8: *C = load &$this
  // CHECK:   n9: *HackMixed = load &$a
  // CHECK:   n10: *HackMixed = load &$b
  // CHECK:   n11 = n8.?.memometh_inst$memoize_impl(n9, n10)
  // CHECK:   n12: *HackMixed = load &$0
  // CHECK:   n13: *HackMixed = load &$1
  // CHECK:   n14 = $builtins.hhbc_memo_set(&memocache::_C_2ememometh__inst, n8, n12, n13, n11)
  // CHECK:   ret n14
  // CHECK: }
  <<__Memoize>>
  public function memometh_inst(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-BAL: define C$static.memometh_static
  // CHECK: define C$static.memometh_static($this: *C$static, $a: *HackInt, $b: *HackInt) : *HackInt {
  // CHECK: local memocache::_C$static_2ememometh__static: *void, $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2: *HackMixed = load &$b
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK:   store &$1 <- n3: *HackMixed
  // CHECK:   n4: *HackMixed = load &$this
  // CHECK:   n5 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__static, n4, n1, n3)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n5)
  // CHECK:   n6 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__static, n4, n1, n3)
  // CHECK:   ret n6
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n5)
  // CHECK:   n7: *HackMixed = load &$a
  // CHECK:   n8: *HackMixed = load &$b
  // CHECK:   n9: *C$static = load &$this
  // CHECK:   n10 = C$static.memometh_static$memoize_impl(n9, n7, n8)
  // CHECK:   n11: *HackMixed = load &$0
  // CHECK:   n12: *HackMixed = load &$1
  // CHECK:   n13 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__static, n9, n11, n12, n10)
  // CHECK:   ret n13
  // CHECK: }
  <<__Memoize>>
  public static function memometh_static(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-1: define C$static.memometh_static$memoize_impl
  // CHECK: define C$static.memometh_static$memoize_impl($this: *C$static, $a: *HackInt, $b: *HackInt) : *HackInt {

  // TEST-CHECK-BAL: define C$static.memometh_lsb
  // CHECK: define C$static.memometh_lsb($this: *C$static, $a: *HackInt, $b: *HackInt) : *HackInt {
  // CHECK: local memocache::_C$static_2ememometh__lsb: *void, $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2: *HackMixed = load &$b
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK:   store &$1 <- n3: *HackMixed
  // CHECK:   n4: *HackMixed = load &$this
  // CHECK:   n5 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__lsb, n4, n1, n3)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true(n5)
  // CHECK:   n6 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__lsb, n4, n1, n3)
  // CHECK:   ret n6
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n5)
  // CHECK:   n7: *HackMixed = load &$a
  // CHECK:   n8: *HackMixed = load &$b
  // CHECK:   n9: *C$static = load &$this
  // CHECK:   n10 = C$static.memometh_lsb$memoize_impl(n9, n7, n8)
  // CHECK:   n11: *HackMixed = load &$0
  // CHECK:   n12: *HackMixed = load &$1
  // CHECK:   n13 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__lsb, n9, n11, n12, n10)
  // CHECK:   ret n13
  // CHECK: }
  <<__MemoizeLSB>>
  public static function memometh_lsb(int $a, int $b)[]: int {
    return $a + $b;
  }
}

// TEST-CHECK-1: define C$static.memometh_lsb$memoize_impl
// CHECK: define C$static.memometh_lsb$memoize_impl($this: *C$static, $a: *HackInt, $b: *HackInt) : *HackInt {

// TEST-CHECK-BAL: define $root.memofunc
// CHECK: define $root.memofunc($this: *void, $a: *HackInt, $b: *HackInt) : *HackInt {
// CHECK: local memocache::_$root_2ememofunc: *void, $0: *void, $1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
// CHECK:   store &$1 <- n3: *HackMixed
// CHECK:   n4 = $builtins.hack_memo_isset(&memocache::_$root_2ememofunc, n1, n3)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   n5 = $builtins.hack_memo_get(&memocache::_$root_2ememofunc, n1, n3)
// CHECK:   ret n5
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   n6: *HackMixed = load &$a
// CHECK:   n7: *HackMixed = load &$b
// CHECK:   n8 = $root.memofunc$memoize_impl(null, n6, n7)
// CHECK:   n9: *HackMixed = load &$0
// CHECK:   n10: *HackMixed = load &$1
// CHECK:   n11 = $builtins.hhbc_memo_set(&memocache::_$root_2ememofunc, n9, n10, n8)
// CHECK:   ret n11
// CHECK: }

<<__Memoize>>
function memofunc(int $a, int $b)[]: int {
  return $a + $b;
}

// TEST-CHECK-1: define $root.memofunc$memoize_impl
// CHECK: define $root.memofunc$memoize_impl($this: *void, $a: *HackInt, $b: *HackInt) : *HackInt {

// TEST-CHECK-BAL: define .async $root.memo_async_func
// CHECK: define .async $root.memo_async_func($this: *void, $a: *HackInt, $b: *HackInt) : *HackInt {
// CHECK: local memocache::_$root_2ememo__async__func: *void, $0: *void, $1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
// CHECK:   store &$1 <- n3: *HackMixed
// CHECK:   n4 = $builtins.hack_memo_isset(&memocache::_$root_2ememo__async__func, n1, n3)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   n5 = $builtins.hack_memo_get(&memocache::_$root_2ememo__async__func, n1, n3)
// CHECK:   ret n5
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   n6: *HackMixed = load &$a
// CHECK:   n7: *HackMixed = load &$b
// CHECK:   n8 = $root.memo_async_func$memoize_impl(null, n6, n7)
// CHECK:   n9 = $builtins.hhbc_await(n8)
// CHECK:   n10: *HackMixed = load &$0
// CHECK:   n11: *HackMixed = load &$1
// CHECK:   n12 = $builtins.hhbc_memo_set(&memocache::_$root_2ememo__async__func, n10, n11, n9)
// CHECK:   ret n12
// CHECK: }

<<__Memoize>>
async function memo_async_func(int $a, int $b)[]: Awaitable<int> {
  return $a + $b;
}

// TEST-CHECK-1: define .async $root.memo_async_func$memoize_impl
// CHECK: define .async $root.memo_async_func$memoize_impl($this: *void, $a: *HackInt, $b: *HackInt) : *HackInt {
