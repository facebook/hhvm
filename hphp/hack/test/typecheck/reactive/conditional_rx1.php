<?hh // partial

<<__RxLocal, __AtMostRxAsArgs>>
async function f(
  <<__AtMostRxAsFunc>>(function(): Awaitable<int>) $async_func,
): Awaitable<int> {
  // OK
  $r = await $async_func();
  return g($r);
}

function g(int $a) {
  return $a;
}
