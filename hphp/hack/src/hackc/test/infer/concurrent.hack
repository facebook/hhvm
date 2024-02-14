// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

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
  // CHECK: define .async $root.Concurrent::genTest1($this: *void) : .awaitable *HackString {
  // CHECK: local $f1: *void, $f2: *void
  // CHECK: #b0:
  // CHECK:   n0 = $root.Concurrent::genGetInt(null)
  // CHECK:   n1 = $builtins.hhbc_await(n0)
  // CHECK:   store &$f1 <- n1: *HackMixed
  // CHECK:   n2 = $root.Concurrent::genVoid1(null)
  // CHECK:   n3 = $builtins.hhbc_await(n2)
  // CHECK:   n4 = $root.Concurrent::genGetString(null)
  // CHECK:   n5 = $builtins.hhbc_await(n4)
  // CHECK:   store &$f2 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$f1
  // CHECK:   n7: *HackMixed = load &$f2
  // CHECK:   n8 = $builtins.hhbc_concat(n7, n6)
  // CHECK:   n9 = $builtins.hhbc_is_type_str(n8)
  // CHECK:   n10 = $builtins.hhbc_verify_type_pred(n8, n9)
  // CHECK:   ret n8
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
  // CHECK: define .async $root.Concurrent::genTest2($this: *void) : .awaitable *HackMixed {
  // CHECK: #b0:
  // CHECK:   n0 = $root.Concurrent::genVoid1(null)
  // CHECK:   n1 = $builtins.hhbc_await(n0)
  // CHECK:   n2 = $root.Concurrent::genVoid2(null)
  // CHECK:   n3 = $builtins.hhbc_await(n2)
  // CHECK:   ret null
  // CHECK: }
  async function genTest2(): Awaitable<void> {
    concurrent {
      await genVoid1();
      await genVoid2();
    }
  }

  // TEST-CHECK-BAL: define .async $root.Concurrent::genTest3
  // CHECK: define .async $root.Concurrent::genTest3($this: *void) : .awaitable *HackInt {
  // CHECK: local $x: *void, $y: *void, $z: *void, $0: *void
  // CHECK: #b0:
  // CHECK:   n0 = $root.Concurrent::genGetVecInt(null)
  // CHECK:   n1 = $builtins.hhbc_await(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n2 = $builtins.hack_int(1)
  // CHECK:   n3: *HackMixed = load &$0
  // CHECK:   n4 = $builtins.hack_array_get(n3, n2)
  // CHECK:   store &$y <- n4: *HackMixed
  // CHECK:   n5 = $builtins.hack_int(0)
  // CHECK:   n6: *HackMixed = load &$0
  // CHECK:   n7 = $builtins.hack_array_get(n6, n5)
  // CHECK:   store &$x <- n7: *HackMixed
  // CHECK:   jmp b3
  // CHECK:   .handlers b2
  // CHECK: #b2(n8: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   throw n8
  // CHECK: #b3:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   n9 = $root.Concurrent::genGetInt(null)
  // CHECK:   n10 = $builtins.hhbc_await(n9)
  // CHECK:   store &$z <- n10: *HackMixed
  // CHECK:   n11: *HackMixed = load &$y
  // CHECK:   n12: *HackMixed = load &$x
  // CHECK:   n13 = $builtins.hhbc_add(n12, n11)
  // CHECK:   n14: *HackMixed = load &$z
  // CHECK:   n15 = $builtins.hhbc_add(n13, n14)
  // CHECK:   n16 = $builtins.hhbc_is_type_int(n15)
  // CHECK:   n17 = $builtins.hhbc_verify_type_pred(n15, n16)
  // CHECK:   ret n15
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
  // CHECK: define .async $root.Concurrent::genTest4($this: *void) : .awaitable *HackString {
  // CHECK: local $x: *void, $y: *void, $0: *void
  // CHECK: #b0:
  // CHECK:   n0 = $root.Concurrent::genGetInt(null)
  // CHECK:   n1 = $builtins.hhbc_await(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n2: *HackMixed = load &$0
  // CHECK:   n3 = $root.Concurrent::intToString(null, n2)
  // CHECK:   store &$x <- n3: *HackMixed
  // CHECK:   jmp b3
  // CHECK:   .handlers b2
  // CHECK: #b2(n4: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   throw n4
  // CHECK: #b3:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   n5 = $root.Concurrent::genGetString(null)
  // CHECK:   n6 = $builtins.hhbc_await(n5)
  // CHECK:   store &$y <- n6: *HackMixed
  // CHECK:   n7: *HackMixed = load &$y
  // CHECK:   n8: *HackMixed = load &$x
  // CHECK:   n9 = $builtins.hhbc_concat(n8, n7)
  // CHECK:   n10 = $builtins.hhbc_is_type_str(n9)
  // CHECK:   n11 = $builtins.hhbc_verify_type_pred(n9, n10)
  // CHECK:   ret n9
  // CHECK: }
  async function genTest4(): Awaitable<string> {
    concurrent {
      $x = intToString(await genGetInt());
      $y = await genGetString();
    }

    return $x.$y;
  }
}
