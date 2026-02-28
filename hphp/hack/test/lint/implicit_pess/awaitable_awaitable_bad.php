<?hh

async function awaitable_awaitable_bad1(int $x): Awaitable<void> {
    $_ = await HH\Lib\Vec\map_async(
      vec[],
      // should not trigger ASYNC_AWAIT_LAMBDA, but should trigger AWAITABLE_AWAITABLE
      async $x ==> awaitable_awaitable_bad1($x), // should trigger rule
    );
}

async function awaitable_awaitable_bad2(int $x): Awaitable<void> {
  $f = async (int $x, int $y) ==> awaitable_awaitable_bad2($x + $y);
  $_ = await $f(0, 0);
}
