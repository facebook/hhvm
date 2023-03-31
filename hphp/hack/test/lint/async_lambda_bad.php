<?hh

async function async_lambda_bad1(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    $x ==> async_lambda_bad1($x),
  );
}

async function async_lambda_bad2(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    $x ==> { return async_lambda_bad2($x); },
  );
}

async function async_lambda_bad3(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    function($x) { return async_lambda_bad3($x); }
  );
}
