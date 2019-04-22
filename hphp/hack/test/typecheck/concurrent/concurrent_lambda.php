<?hh // strict

async function run<T>(
  (function (int): Awaitable<T>) $x,
): Awaitable<T> {
  return await $x(42);
}
async function gen<T>(T $x): Awaitable<T> { return $x; }

async function foo(): Awaitable<int> {
  concurrent {
    $x1 = await run(async ($a) ==> await gen($a));
    $x2 = await run(async ($a) ==> { return await gen($a); });
    $x3 = await run(async function($a) { return await gen($a); });
    $x4 = await async { $a = 42; return await gen($a); };
  }
  return $x1 + $x2 + $x3 + $x4;
}
