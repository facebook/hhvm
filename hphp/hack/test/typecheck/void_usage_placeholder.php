<?hh // partial

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

  /* HH_FIXME[4119] even though void is subtype of Tmixed */
  takes_mixed($_);

  $arr = array();
  foreach ($arr as $k => $_) {
    /* HH_FIXME[4119] even though void is a subtype of Tany */
    takes_any($_);
  }
}

function takes_mixed(mixed $m): void {}

function takes_any($any): void {}

function test_unknown_usage(): void {
  undefined_function($_);
}
