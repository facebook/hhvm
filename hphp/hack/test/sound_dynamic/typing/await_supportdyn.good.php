<?hh

class C { }

function expect_dynamic(dynamic $_):void { }

async function testit((function():supportdyn<Awaitable<C>>) $f, (function():Awaitable<supportdyn<C>>) $g):Awaitable<void> {
  $y = await $g();
  expect_dynamic($y);
  $x = await $f();
  expect_dynamic($x);
}
