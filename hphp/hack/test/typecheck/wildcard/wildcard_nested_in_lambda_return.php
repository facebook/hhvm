<?hh

function test1(): int {
  $f = (int $x): vec<_> ==> vec[$x];
  $x = $f(3);
  return $x[0];
}

async function test2(): Awaitable<int> {
  $f = async (int $x): Awaitable<vec<_>> ==> vec[$x];
  $x = await $f(3);
  return $x[0];
}

<<__EntryPoint>>
function main(): void {
  test1();
}

async function test3(): Awaitable<int> {
  $f = async (int $x): Awaitable<_> ==> $x;
  $x = await $f(3);
  return $x;
}
