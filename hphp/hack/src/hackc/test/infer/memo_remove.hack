// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

class C {
  // TEST-CHECK-BAL: define C.memometh_inst
  // CHECK: define C.memometh_inst($this: *C, $a: *HackInt, $b: *HackInt) : *HackInt {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$b
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK:   n2 = $builtins.hhbc_add(n1, n0)
  // CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
  // CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
  // CHECK:   ret n2
  // CHECK: }
  <<__Memoize>>
  public function memometh_inst(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-BAL: define C$static.memometh_static
  // CHECK: define C$static.memometh_static($this: *C$static, $a: *HackInt, $b: *HackInt) : *HackInt {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$b
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK:   n2 = $builtins.hhbc_add(n1, n0)
  // CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
  // CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
  // CHECK:   ret n2
  // CHECK: }
  <<__Memoize>>
  public static function memometh_static(int $a, int $b)[]: int {
    return $a + $b;
  }

  // TEST-CHECK-BAL: define C$static.memometh_lsb
  // CHECK: define C$static.memometh_lsb($this: *C$static, $a: *HackInt, $b: *HackInt) : *HackInt {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$b
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK:   n2 = $builtins.hhbc_add(n1, n0)
  // CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
  // CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
  // CHECK:   ret n2
  // CHECK: }
  <<__MemoizeLSB>>
  public static function memometh_lsb(int $a, int $b)[]: int {
    return $a + $b;
  }
}

// TEST-CHECK-BAL: define $root.memofunc
// CHECK: define $root.memofunc($this: *void, $a: *HackInt, $b: *HackInt) : *HackInt {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$b
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_add(n1, n0)
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK:   ret n2
// CHECK: }

<<__Memoize>>
function memofunc(int $a, int $b)[]: int {
  return $a + $b;
}

// TEST-CHECK-BAL: define .async $root.memo_async_func
// CHECK: define .async $root.memo_async_func($this: *void, $a: *HackInt, $b: *HackInt) : *HackInt {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$b
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_add(n1, n0)
// CHECK:   n3 = $builtins.hhbc_is_type_int(n2)
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK:   ret n2
// CHECK: }

<<__Memoize>>
async function memo_async_func(int $a, int $b)[]: Awaitable<int> {
  return $a + $b;
}
