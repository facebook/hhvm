// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define .async $root.test_async
// CHECK: define .async $root.test_async($this: *void) : .awaitable *HackInt {
// CHECK: local $a: *void, $a2: *void, $b: *void
// CHECK: #b0:
// CHECK:   n0 = $root.bar(null)
// CHECK:   n1 = $builtins.hhbc_await(n0)
// CHECK:   store &$a <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$a
// CHECK:   n3 = $root.baz(null, n2)
// CHECK:   n4 = $builtins.hhbc_await(n3)
// CHECK:   store &$b <- n4: *HackMixed
// CHECK:   n5 = $root.bar(null)
// CHECK:   store &$a2 <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$a2
// CHECK:   n7 = $builtins.hhbc_is_type_null(n6)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   jmp b3(n6)
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   n8 = $builtins.hhbc_await(n6)
// CHECK:   jmp b3(n8)
// CHECK: #b3(n9: *HackMixed):
// CHECK:   n10: *HackMixed = load &$b
// CHECK:   n11 = $builtins.hhbc_is_type_int(n10)
// CHECK:   n12 = $builtins.hhbc_verify_type_pred(n10, n11)
// CHECK:   ret n10
// CHECK: }
async function test_async(): Awaitable<int> {
  $a = await bar();
  $b = await baz($a);
  $a2 = bar();
  await $a2;
  return $b;
}

async function bar(): Awaitable<int> { return 5; }
async function baz(int $a): Awaitable<int> { return 6; }
