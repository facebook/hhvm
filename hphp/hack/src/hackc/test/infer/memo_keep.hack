// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast --keep-memo %s | FileCheck %s

class C {
  // TEST-CHECK-BAL: define C.memometh_inst
  // CHECK: define C.memometh_inst($this: .notnull *C, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C_2ememometh__inst: *void, $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK: // .column 10
  // CHECK:   n0: *HackMixed = load &$this
  // CHECK: // .column 10
  // CHECK:   n1 = $builtins.hhbc_check_this(n0)
  // CHECK: // .column 10
  // CHECK:   n2: *HackMixed = load &$a
  // CHECK: // .column 10
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK: // .column 10
  // CHECK:   store &$0 <- n3: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n4: *HackMixed = load &$b
  // CHECK: // .column 10
  // CHECK:   n5 = $builtins.hhbc_get_memo_key_l(n4)
  // CHECK: // .column 10
  // CHECK:   store &$1 <- n5: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n6: *HackMixed = load &$this
  // CHECK: // .column 10
  // CHECK:   n7: *HackMixed = load &$0
  // CHECK: // .column 10
  // CHECK:   n8: *HackMixed = load &$1
  // CHECK: // .column 10
  // CHECK:   n9 = $builtins.hack_memo_isset(&memocache::_C_2ememometh__inst, n6, n7, n8)
  // CHECK: // .column 10
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK: // .column 10
  // CHECK:   prune $builtins.hack_is_true(n9)
  // CHECK: // .column 10
  // CHECK:   n10 = $builtins.hack_memo_get(&memocache::_C_2ememometh__inst, n6, n7, n8)
  // CHECK: // .column 10
  // CHECK:   ret n10
  // CHECK: #b2:
  // CHECK: // .column 10
  // CHECK:   prune ! $builtins.hack_is_true(n9)
  // CHECK: // .column 10
  // CHECK:   n11: *C = load &$this
  // CHECK: // .column 10
  // CHECK:   n12: *HackMixed = load &$a
  // CHECK: // .column 10
  // CHECK:   n13: *HackMixed = load &$b
  // CHECK: // .column 10
  // CHECK:   n14 = n11.?.memometh_inst$memoize_impl(n12, n13)
  // CHECK: // .column 10
  // CHECK:   n15: *HackMixed = load &$this
  // CHECK: // .column 10
  // CHECK:   n16: *HackMixed = load &$0
  // CHECK: // .column 10
  // CHECK:   n17: *HackMixed = load &$1
  // CHECK: // .column 10
  // CHECK:   n18 = $builtins.hhbc_memo_set(&memocache::_C_2ememometh__inst, n15, n16, n17, n14)
  // CHECK: // .column 10
  // CHECK:   ret n18
  // CHECK: }
  <<__Memoize>>
  public function memometh_inst(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-BAL: define C$static.memometh_static
  // CHECK: define C$static.memometh_static($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C$static_2ememometh__static: *void, $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK: // .column 17
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK: // .column 17
  // CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
  // CHECK: // .column 17
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK: // .column 17
  // CHECK:   n2: *HackMixed = load &$b
  // CHECK: // .column 17
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK: // .column 17
  // CHECK:   store &$1 <- n3: *HackMixed
  // CHECK: // .column 17
  // CHECK:   n4: *HackMixed = load &$this
  // CHECK: // .column 17
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK: // .column 17
  // CHECK:   n6: *HackMixed = load &$1
  // CHECK: // .column 17
  // CHECK:   n7 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__static, n4, n5, n6)
  // CHECK: // .column 17
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK: // .column 17
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK: // .column 17
  // CHECK:   n8 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__static, n4, n5, n6)
  // CHECK: // .column 17
  // CHECK:   ret n8
  // CHECK: #b2:
  // CHECK: // .column 17
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK: // .column 17
  // CHECK:   n9: *HackMixed = load &$a
  // CHECK: // .column 17
  // CHECK:   n10: *HackMixed = load &$b
  // CHECK: // .column 17
  // CHECK:   n11 = __sil_lazy_class_initialize(<C>)
  // CHECK:   n12 = C$static.memometh_static$memoize_impl(n11, n9, n10)
  // CHECK: // .column 17
  // CHECK:   n13: *HackMixed = load &$this
  // CHECK: // .column 17
  // CHECK:   n14: *HackMixed = load &$0
  // CHECK: // .column 17
  // CHECK:   n15: *HackMixed = load &$1
  // CHECK: // .column 17
  // CHECK:   n16 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__static, n13, n14, n15, n12)
  // CHECK: // .column 17
  // CHECK:   ret n16
  // CHECK: }
  <<__Memoize>>
  public static function memometh_static(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-1: define C$static.memometh_static$memoize_impl
  // CHECK: define C$static.memometh_static$memoize_impl($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {

  // TEST-CHECK-BAL: define C$static.memometh_lsb
  // CHECK: define C$static.memometh_lsb($this: .notnull *C$static, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {
  // CHECK: local memocache::_C$static_2ememometh__lsb: *void, $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK: // .column 17
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK: // .column 17
  // CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
  // CHECK: // .column 17
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK: // .column 17
  // CHECK:   n2: *HackMixed = load &$b
  // CHECK: // .column 17
  // CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
  // CHECK: // .column 17
  // CHECK:   store &$1 <- n3: *HackMixed
  // CHECK: // .column 17
  // CHECK:   n4: *HackMixed = load &$this
  // CHECK: // .column 17
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK: // .column 17
  // CHECK:   n6: *HackMixed = load &$1
  // CHECK: // .column 17
  // CHECK:   n7 = $builtins.hack_memo_isset(&memocache::_C$static_2ememometh__lsb, n4, n5, n6)
  // CHECK: // .column 17
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK: // .column 17
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK: // .column 17
  // CHECK:   n8 = $builtins.hack_memo_get(&memocache::_C$static_2ememometh__lsb, n4, n5, n6)
  // CHECK: // .column 17
  // CHECK:   ret n8
  // CHECK: #b2:
  // CHECK: // .column 17
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK: // .column 17
  // CHECK:   n9: *HackMixed = load &$a
  // CHECK: // .column 17
  // CHECK:   n10: *HackMixed = load &$b
  // CHECK: // .column 17
  // CHECK:   n11: *C$static = load &$this
  // CHECK:   n12 = C$static.memometh_lsb$memoize_impl(n11, n9, n10)
  // CHECK: // .column 17
  // CHECK:   n13: *HackMixed = load &$this
  // CHECK: // .column 17
  // CHECK:   n14: *HackMixed = load &$0
  // CHECK: // .column 17
  // CHECK:   n15: *HackMixed = load &$1
  // CHECK: // .column 17
  // CHECK:   n16 = $builtins.hhbc_memo_set(&memocache::_C$static_2ememometh__lsb, n13, n14, n15, n12)
  // CHECK: // .column 17
  // CHECK:   ret n16
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
// CHECK: local memocache::_$root_2ememofunc: *void, $0: *void, $1: *void
// CHECK: #b0:
// CHECK: // .column 1
// CHECK:   n0: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
// CHECK: // .column 1
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 1
// CHECK:   n2: *HackMixed = load &$b
// CHECK: // .column 1
// CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
// CHECK: // .column 1
// CHECK:   store &$1 <- n3: *HackMixed
// CHECK: // .column 1
// CHECK:   n4: *HackMixed = load &$0
// CHECK: // .column 1
// CHECK:   n5: *HackMixed = load &$1
// CHECK: // .column 1
// CHECK:   n6 = $builtins.hack_memo_isset(&memocache::_$root_2ememofunc, n4, n5)
// CHECK: // .column 1
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 1
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK: // .column 1
// CHECK:   n7 = $builtins.hack_memo_get(&memocache::_$root_2ememofunc, n4, n5)
// CHECK: // .column 1
// CHECK:   ret n7
// CHECK: #b2:
// CHECK: // .column 1
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK: // .column 1
// CHECK:   n8: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n9: *HackMixed = load &$b
// CHECK: // .column 1
// CHECK:   n10 = $root.memofunc$memoize_impl(null, n8, n9)
// CHECK: // .column 1
// CHECK:   n11: *HackMixed = load &$0
// CHECK: // .column 1
// CHECK:   n12: *HackMixed = load &$1
// CHECK: // .column 1
// CHECK:   n13 = $builtins.hhbc_memo_set(&memocache::_$root_2ememofunc, n11, n12, n10)
// CHECK: // .column 1
// CHECK:   ret n13
// CHECK: }

<<__Memoize>>
function memofunc(int $a, int $b)[]: int {
  return $a + $b;
}

// TEST-CHECK-1: define $root.memofunc$memoize_impl
// CHECK: define $root.memofunc$memoize_impl($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : .notnull *HackInt {

// TEST-CHECK-BAL: define .async $root.memo_async_func
// CHECK: define .async $root.memo_async_func($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : .awaitable .notnull *HackInt {
// CHECK: local memocache::_$root_2ememo__async__func: *void, $0: *void, $1: *void
// CHECK: #b0:
// CHECK: // .column 7
// CHECK:   n0: *HackMixed = load &$a
// CHECK: // .column 7
// CHECK:   n1 = $builtins.hhbc_get_memo_key_l(n0)
// CHECK: // .column 7
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 7
// CHECK:   n2: *HackMixed = load &$b
// CHECK: // .column 7
// CHECK:   n3 = $builtins.hhbc_get_memo_key_l(n2)
// CHECK: // .column 7
// CHECK:   store &$1 <- n3: *HackMixed
// CHECK: // .column 7
// CHECK:   n4: *HackMixed = load &$0
// CHECK: // .column 7
// CHECK:   n5: *HackMixed = load &$1
// CHECK: // .column 7
// CHECK:   n6 = $builtins.hack_memo_isset(&memocache::_$root_2ememo__async__func, n4, n5)
// CHECK: // .column 7
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 7
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK: // .column 7
// CHECK:   n7 = $builtins.hack_memo_get(&memocache::_$root_2ememo__async__func, n4, n5)
// CHECK: // .column 7
// CHECK:   ret n7
// CHECK: #b2:
// CHECK: // .column 7
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK: // .column 7
// CHECK:   n8: *HackMixed = load &$a
// CHECK: // .column 7
// CHECK:   n9: *HackMixed = load &$b
// CHECK: // .column 7
// CHECK:   n10 = $root.memo_async_func$memoize_impl(null, n8, n9)
// CHECK:   n11 = $builtins.hhbc_await(n10)
// CHECK: // .column 7
// CHECK:   n12: *HackMixed = load &$0
// CHECK: // .column 7
// CHECK:   n13: *HackMixed = load &$1
// CHECK: // .column 7
// CHECK:   n14 = $builtins.hhbc_memo_set(&memocache::_$root_2ememo__async__func, n12, n13, n11)
// CHECK: // .column 7
// CHECK:   ret n14
// CHECK: }

<<__Memoize>>
async function memo_async_func(int $a, int $b)[]: Awaitable<int> {
  return $a + $b;
}

// TEST-CHECK-1: define .async $root.memo_async_func$memoize_impl
// CHECK: define .async $root.memo_async_func$memoize_impl($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : .awaitable .notnull *HackInt {


// TEST-CHECK-BAL: define $root.memo_with_pure_zero_param($this: *void) : .notnull *HackInt {
// CHECK: define $root.memo_with_pure_zero_param($this: *void) : .notnull *HackInt {
// CHECK: local memocache::_$root_2ememo__with__pure__zero__param: *void
// CHECK: #b0:
// CHECK: // .column 1
// CHECK:   n0 = $builtins.hack_memo_isset(&memocache::_$root_2ememo__with__pure__zero__param)
// CHECK: // .column 1
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 1
// CHECK:   prune $builtins.hack_is_true(n0)
// CHECK: // .column 1
// CHECK:   n1 = $builtins.hack_memo_get(&memocache::_$root_2ememo__with__pure__zero__param)
// CHECK: // .column 1
// CHECK:   ret n1
// CHECK: #b2:
// CHECK: // .column 1
// CHECK:   prune ! $builtins.hack_is_true(n0)
// CHECK: // .column 1
// CHECK:   n2 = $root.memo_with_pure_zero_param$memoize_impl(null)
// CHECK: // .column 1
// CHECK:   n3 = $builtins.hhbc_memo_set(&memocache::_$root_2ememo__with__pure__zero__param, n2)
// CHECK: // .column 1
// CHECK:   ret n3
// CHECK: }


<<__Memoize>>
function memo_with_pure_zero_param()[]: int {
  return 42;
}
