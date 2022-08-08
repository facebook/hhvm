<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function expect<T as supportdyn<mixed>>(supportdyn<(function():~Awaitable<vec<T>>)> $f):void { }
function expect2<T as supportdyn<mixed> >((function():~Awaitable<vec<T>>) $_):void { }

function any(
    supportdyn<(function(): Awaitable<~vec<int>>)> $fn1,
    supportdyn<(function(): ~Awaitable<vec<int>>)> $fn2,
    (function(): ~Awaitable<vec<int>>) $fn3,
  ): void {
  // This works
  expect($fn1); expect2($fn1);
  // This also works
  $f = <<__SupportDynamicType>> async () ==>  { return await $fn2(); };
  expect($f);
  expect2($f);
  // This does not work
  expect(async () ==>  { return await $fn2(); } );
  expect2(async () ==>  { return await $fn2(); } );
}
