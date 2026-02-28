<?hh

async function async_lambda_good1(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    async $x ==> await async_lambda_good1($x),
  );
}

async function async_lambda_good2(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    async $x ==> async_lambda_good2($x),
  );
}

async function async_lambda_good3(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    async $x ==> { return await async_lambda_good3($x); },
  );
}

function async_lambda_good4(int $x): void {
  $_ = HH\Lib\Vec\map(
    vec[],
    $x ==> async_lambda_good4($x),
  );
}

async function async_lambda_good5(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    async function($x) { return await async_lambda_good5($x); }
  );
}
