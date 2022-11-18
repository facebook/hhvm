// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: define $root.test_async(this: *void) : *HackMixed {
// CHECK: local $a: *void, $b: *void, base: *HackMixed
// CHECK: #b0:
// CHECK:   n0 = $root.bar(null)
// CHECK:   n1 = $builtins.await(n0)
// CHECK:   store &$a <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$a
// CHECK:   n3 = $root.baz(null, n2)
// CHECK:   n4 = $builtins.await(n3)
// CHECK:   store &$b <- n4: *HackMixed
// ...
// CHECK:   ret n5
// CHECK: }
async function test_async(): Awaitable<int> {
  $a = await bar();
  $b = await baz($a);
  return $b;
}

async function bar(): Awaitable<int> { return 5; }
async function baz(int $a): Awaitable<int> { return 6; }
