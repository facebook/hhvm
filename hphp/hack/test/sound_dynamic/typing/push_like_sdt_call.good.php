<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C<T as supportdyn<mixed>> { }

<<__SupportDynamicType>>
class Aw<T as supportdyn<mixed>> {}

function bar(): Aw<~C<int>> {
  throw new Exception("A");
}

<<__SupportDynamicType>>
function foo<T as supportdyn<mixed>>(Aw<C<T>> $gen_value): void { }

function testit(): void {
  // Succeeds
  foo<int>(bar());
  // Fails
  foo(bar());
}
