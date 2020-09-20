<?hh // strict

async function gen_int(): Awaitable<int> {
  return 1;
}

async function test(): Awaitable<int> {
  concurrent {
    $v1 = await gen_int();
    $v2 = gen_int();
  }
  return $v1 + $v2;
}
