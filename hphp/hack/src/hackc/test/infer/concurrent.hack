// RUN: %hackc compile-infer --unwrap-concurrent %s | FileCheck %s

namespace Concurrent {
  async function genGetInt(): Awaitable<int> {
    return 42;
  }

  async function genGetString(): Awaitable<string> {
    return "Hello";
  }

  async function genVoid1(): Awaitable<void> {
    return;
  }

  async function genVoid2(): Awaitable<void> {
    return;
  }

  async function genGetVecInt(): Awaitable<vec<int>> {
    return vec[23, 42];
  }

  // TEST-CHECK-BAL: define .async $root.Concurrent::genTest1
  // CHECK: define .async $root.Concurrent::genTest1($this: *void) : *HackString {
  // CHECK: local $f1: *void, $f2: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0 = $root.Concurrent::genGetInt(null)
  // CHECK:   store &$0 <- n0: *HackMixed
  // CHECK:   n1 = $root.Concurrent::genVoid1(null)
  // CHECK:   store &$1 <- n1: *HackMixed
  // CHECK:   n2 = $root.Concurrent::genGetString(null)
  // CHECK:   store &$2 <- n2: *HackMixed
  // CHECK:   n3 = $builtins.hhbc_await_all(n0, n1, n2)
  // CHECK:   n4 = $builtins.hhbc_is_type_null(n0)
  // CHECK:   jmp b2, b3
  // CHECK:   .handlers b11
  // CHECK: #b2:
  // CHECK:   prune $builtins.hack_is_true(n4)
  // CHECK:   jmp b4(n0)
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n4)
  // CHECK:   n5 = $builtins.hhbc_wh_result(n0)
  // CHECK:   jmp b4(n5)
  // CHECK:   .handlers b11
  // CHECK: #b4(n6: *HackMixed):
  // CHECK:   store &$0 <- n6: *HackMixed
  // CHECK:   n7: *HackMixed = load &$1
  // CHECK:   n8 = $builtins.hhbc_is_type_null(n7)
  // CHECK:   jmp b5, b6
  // CHECK:   .handlers b11
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n8)
  // CHECK:   jmp b7(n7)
  // CHECK: #b6:
  // CHECK:   prune ! $builtins.hack_is_true(n8)
  // CHECK:   n9 = $builtins.hhbc_wh_result(n7)
  // CHECK:   jmp b7(n9)
  // CHECK:   .handlers b11
  // CHECK: #b7(n10: *HackMixed):
  // CHECK:   store &$1 <- n10: *HackMixed
  // CHECK:   n11: *HackMixed = load &$2
  // CHECK:   n12 = $builtins.hhbc_is_type_null(n11)
  // CHECK:   jmp b8, b9
  // CHECK:   .handlers b11
  // CHECK: #b8:
  // CHECK:   prune $builtins.hack_is_true(n12)
  // CHECK:   jmp b10(n11)
  // CHECK: #b9:
  // CHECK:   prune ! $builtins.hack_is_true(n12)
  // CHECK:   n13 = $builtins.hhbc_wh_result(n11)
  // CHECK:   jmp b10(n13)
  // CHECK:   .handlers b11
  // CHECK: #b10(n14: *HackMixed):
  // CHECK:   store &$2 <- n14: *HackMixed
  // CHECK:   n15: *HackMixed = load &$0
  // CHECK:   store &$f1 <- n15: *HackMixed
  // CHECK:   store &$f2 <- n14: *HackMixed
  // CHECK:   jmp b12
  // CHECK:   .handlers b11
  // CHECK: #b11(n16: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n17 = $builtins.hhbc_throw(n16)
  // CHECK:   unreachable
  // CHECK: #b12:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n18: *HackMixed = load &$f1
  // CHECK:   n19: *HackMixed = load &$f2
  // CHECK:   n20 = $builtins.hhbc_concat(n19, n18)
  // CHECK:   n21 = $builtins.hhbc_is_type_str(n20)
  // CHECK:   n22 = $builtins.hhbc_verify_type_pred(n20, n21)
  // CHECK:   ret n20
  // CHECK: }
  async function genTest1(): Awaitable<string> {
    concurrent {
      $f1 = await genGetInt();
      await genVoid1();
      $f2 = await genGetString();
    }

    return $f2.$f1;
  }

  // TEST-CHECK-BAL: define .async $root.Concurrent::genTest2
  // CHECK: define .async $root.Concurrent::genTest2($this: *void) : *HackMixed {
  // CHECK: local $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0 = $root.Concurrent::genVoid1(null)
  // CHECK:   store &$0 <- n0: *HackMixed
  // CHECK:   n1 = $root.Concurrent::genVoid2(null)
  // CHECK:   store &$1 <- n1: *HackMixed
  // CHECK:   n2 = $builtins.hhbc_await_all(n0, n1)
  // CHECK:   n3 = $builtins.hhbc_is_type_null(n0)
  // CHECK:   jmp b2, b3
  // CHECK:   .handlers b8
  // CHECK: #b2:
  // CHECK:   prune $builtins.hack_is_true(n3)
  // CHECK:   jmp b4(n0)
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n3)
  // CHECK:   n4 = $builtins.hhbc_wh_result(n0)
  // CHECK:   jmp b4(n4)
  // CHECK:   .handlers b8
  // CHECK: #b4(n5: *HackMixed):
  // CHECK:   store &$0 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$1
  // CHECK:   n7 = $builtins.hhbc_is_type_null(n6)
  // CHECK:   jmp b5, b6
  // CHECK:   .handlers b8
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   jmp b7(n6)
  // CHECK: #b6:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n8 = $builtins.hhbc_wh_result(n6)
  // CHECK:   jmp b7(n8)
  // CHECK:   .handlers b8
  // CHECK: #b7(n9: *HackMixed):
  // CHECK:   store &$1 <- n9: *HackMixed
  // CHECK:   jmp b9
  // CHECK:   .handlers b8
  // CHECK: #b8(n10: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   n11 = $builtins.hhbc_throw(n10)
  // CHECK:   unreachable
  // CHECK: #b9:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  async function genTest2(): Awaitable<void> {
    concurrent {
      await genVoid1();
      await genVoid2();
    }
  }

  // TEST-CHECK-BAL: define .async $root.Concurrent::genTest3
  // CHECK: define .async $root.Concurrent::genTest3($this: *void) : *HackInt {
  // CHECK: local $x: *void, $y: *void, $z: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0 = $root.Concurrent::genGetVecInt(null)
  // CHECK:   store &$0 <- n0: *HackMixed
  // CHECK:   n1 = $root.Concurrent::genGetInt(null)
  // CHECK:   store &$1 <- n1: *HackMixed
  // CHECK:   n2 = $builtins.hhbc_await_all(n0, n1)
  // CHECK:   n3 = $builtins.hhbc_is_type_null(n0)
  // CHECK:   jmp b2, b3
  // CHECK:   .handlers b11
  // CHECK: #b2:
  // CHECK:   prune $builtins.hack_is_true(n3)
  // CHECK:   jmp b4(n0)
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n3)
  // CHECK:   n4 = $builtins.hhbc_wh_result(n0)
  // CHECK:   jmp b4(n4)
  // CHECK:   .handlers b11
  // CHECK: #b4(n5: *HackMixed):
  // CHECK:   store &$0 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$1
  // CHECK:   n7 = $builtins.hhbc_is_type_null(n6)
  // CHECK:   jmp b5, b6
  // CHECK:   .handlers b11
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   jmp b7(n6)
  // CHECK: #b6:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n8 = $builtins.hhbc_wh_result(n6)
  // CHECK:   jmp b7(n8)
  // CHECK:   .handlers b11
  // CHECK: #b7(n9: *HackMixed):
  // CHECK:   store &$1 <- n9: *HackMixed
  // CHECK:   n10: *HackMixed = load &$0
  // CHECK:   store &$2 <- n10: *HackMixed
  // CHECK:   jmp b8
  // CHECK:   .handlers b11
  // CHECK: #b8:
  // CHECK:   n11 = $builtins.hack_int(1)
  // CHECK:   n12: *HackMixed = load &$2
  // CHECK:   n13 = $builtins.hack_array_get(n12, n11)
  // CHECK:   store &$y <- n13: *HackMixed
  // CHECK:   n14 = $builtins.hack_int(0)
  // CHECK:   n15 = $builtins.hack_array_get(n12, n14)
  // CHECK:   store &$x <- n15: *HackMixed
  // CHECK:   jmp b10
  // CHECK:   .handlers b9
  // CHECK: #b9(n16: *HackMixed):
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n17 = $builtins.hhbc_throw(n16)
  // CHECK:   unreachable
  // CHECK:   .handlers b11
  // CHECK: #b10:
  // CHECK:   n18: *HackMixed = load &$2
  // CHECK:   n19: *HackMixed = load &$1
  // CHECK:   store &$z <- n19: *HackMixed
  // CHECK:   jmp b12
  // CHECK:   .handlers b11
  // CHECK: #b11(n20: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   n21 = $builtins.hhbc_throw(n20)
  // CHECK:   unreachable
  // CHECK: #b12:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   n22: *HackMixed = load &$y
  // CHECK:   n23: *HackMixed = load &$x
  // CHECK:   n24 = $builtins.hhbc_add(n23, n22)
  // CHECK:   n25: *HackMixed = load &$z
  // CHECK:   n26 = $builtins.hhbc_add(n24, n25)
  // CHECK:   n27 = $builtins.hhbc_is_type_int(n26)
  // CHECK:   n28 = $builtins.hhbc_verify_type_pred(n26, n27)
  // CHECK:   ret n26
  // CHECK: }
  async function genTest3(): Awaitable<int> {
    concurrent {
      list($x, $y) = await genGetVecInt();
      $z = await genGetInt();
    }

    return $x + $y + $z;
  }

  function intToString(int $x): string {
    return "".$x;
  }

  // TEST-CHECK-BAL: define .async $root.Concurrent::genTest4
  // CHECK: define .async $root.Concurrent::genTest4($this: *void) : *HackString {
  // CHECK: local $x: *void, $y: *void, $0: *void, $1: *void
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0 = $root.Concurrent::genGetInt(null)
  // CHECK:   store &$0 <- n0: *HackMixed
  // CHECK:   n1 = $root.Concurrent::genGetString(null)
  // CHECK:   store &$1 <- n1: *HackMixed
  // CHECK:   n2 = $builtins.hhbc_await_all(n0, n1)
  // CHECK:   n3 = $builtins.hhbc_is_type_null(n0)
  // CHECK:   jmp b2, b3
  // CHECK:   .handlers b8
  // CHECK: #b2:
  // CHECK:   prune $builtins.hack_is_true(n3)
  // CHECK:   jmp b4(n0)
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n3)
  // CHECK:   n4 = $builtins.hhbc_wh_result(n0)
  // CHECK:   jmp b4(n4)
  // CHECK:   .handlers b8
  // CHECK: #b4(n5: *HackMixed):
  // CHECK:   store &$0 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$1
  // CHECK:   n7 = $builtins.hhbc_is_type_null(n6)
  // CHECK:   jmp b5, b6
  // CHECK:   .handlers b8
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   jmp b7(n6)
  // CHECK: #b6:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n8 = $builtins.hhbc_wh_result(n6)
  // CHECK:   jmp b7(n8)
  // CHECK:   .handlers b8
  // CHECK: #b7(n9: *HackMixed):
  // CHECK:   store &$1 <- n9: *HackMixed
  // CHECK:   n10: *HackMixed = load &$0
  // CHECK:   n11 = $root.Concurrent::intToString(null, n10)
  // CHECK:   store &$0 <- n11: *HackMixed
  // CHECK:   store &$x <- n11: *HackMixed
  // CHECK:   store &$y <- n9: *HackMixed
  // CHECK:   jmp b9
  // CHECK:   .handlers b8
  // CHECK: #b8(n12: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   n13 = $builtins.hhbc_throw(n12)
  // CHECK:   unreachable
  // CHECK: #b9:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   n14: *HackMixed = load &$y
  // CHECK:   n15: *HackMixed = load &$x
  // CHECK:   n16 = $builtins.hhbc_concat(n15, n14)
  // CHECK:   n17 = $builtins.hhbc_is_type_str(n16)
  // CHECK:   n18 = $builtins.hhbc_verify_type_pred(n16, n17)
  // CHECK:   ret n16
  // CHECK: }
  async function genTest4(): Awaitable<string> {
    concurrent {
      $x = intToString(await genGetInt());
      $y = await genGetString();
    }

    return $x.$y;
  }
}
