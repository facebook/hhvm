<?hh

<<__RxLocal, __OnlyRxIfArgs>>
async function f(
  <<__OnlyRxIfRxFunc>>(function(): Awaitable<int>) $async_func,
): Awaitable<int> {
  // OK
  $r = await $async_func();
  return g($r);
}

function g(int $a) {
  return $a;
}
