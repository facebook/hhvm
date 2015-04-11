<?hh // strict

async function yield_int(): Awaitable<int> {
  // UNSAFE
}

async function yield_void(): Awaitable<void> {}

function v(): void {}

async function test(): Awaitable<void> {
  list($x, $_) = await genva(yield_int(), yield_void());
  hh_show($x);
  hh_show($_);
  $_ = v();

  $arr = array();
  foreach ($arr as $k => $_) {
    takes_mixed($_);
  }
}

function takes_mixed(mixed $m): void {}
