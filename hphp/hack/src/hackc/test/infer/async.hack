// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define .async $root.test_async
// CHECK: define .async $root.test_async($this: *void) : *HackInt {
// CHECK: local $a: *void, $a2: *void, $b: *void
// CHECK: #b0:
// CHECK:   n0 = $root.bar(null)
// CHECK:   n1 = $builtins.hhbc_await(n0)
// CHECK:   store &$a <- n1: *HackMixed
// CHECK:   n2 = $root.baz(null, n1)
// CHECK:   n3 = $builtins.hhbc_await(n2)
// CHECK:   store &$b <- n3: *HackMixed
// CHECK:   n4 = $root.bar(null)
// CHECK:   store &$a2 <- n4: *HackMixed
// CHECK:   n5 = $builtins.hhbc_is_type_null(n4)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n5)
// CHECK:   jmp b3(n4)
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n5)
// CHECK:   n6 = $builtins.hhbc_await(n4)
// CHECK:   jmp b3(n6)
// CHECK: #b3(n7: *HackMixed):
// CHECK:   n8: *HackMixed = load &$b
// CHECK:   n9 = $builtins.hhbc_is_type_int(n8)
// CHECK:   n10 = $builtins.hhbc_verify_type_pred(n8, n9)
// CHECK:   ret n8
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
