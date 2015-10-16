<?hh // strict

function f(int $x): int {
  echo "f(",$x,")\n";
  return $x;
}

async function genNum(int $x): Awaitable<int> {
  if ($x == 1) {
    await wait();
  }
  return $x;
}

async function genNum2(int $x, int $y): Awaitable<int> {
  return $x+$y;
}

async function foo0(): Awaitable<int> {
  return await genNum(0);
}

async function foo1(): Awaitable<int> {
  $x = await genNum(0);
  $y = await genNum(1);
  return $x + $y;
}

async function foo2(): Awaitable<int> {
  list($x, $y) = await gena_(array(genNum(f(1)), genNum(f(2))));
  return $x + $y;
}

// We can't actually run this function because we don't have a genva
// implementation...
async function foo2_genva(): Awaitable<int> {
  list($x, $y) = await genva(genNum(f(1)), genNum(f(2)));
  return $x + $y;
}

async function foo3(): Awaitable<array<int, int>> {
  $nus = array();
  $nus[f(0)] = await genNum2(f(1), f(2));
  return $nus;
}

class Foo {
  public async function gen(): Awaitable<int> {
    return await foo0();
  }
}
async function foo4(): Awaitable<int> {
  $x = new Foo();
  return await $x->gen();
}

function prep<T>(Awaitable<T> $aw): T {
  /* HH_FIXME[4053]: WaitHandle missing join in hh_single_type_check */
  return $aw->getWaitHandle()->join();
}

function gena_<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables
  ): Awaitable<array<Tv>> {
  $wait_handles = array();
  foreach ($awaitables as $index => $awaitable) {
    $wait_handles[$index] = $awaitable->getWaitHandle();
  }
  /* HH_FIXME[2049] */
  /* HH_FIXME[4026] */
  return GenArrayWaitHandle::create($wait_handles);
}

function wait(): Awaitable<void> {
  /* HH_FIXME[2049] */
  /* HH_FIXME[4026] */
  return RescheduleWaitHandle::create(0, 0);
}

function test(): void {
  var_dump(prep(foo0()));
  var_dump(prep(foo1()));
  var_dump(prep(foo2()));
  var_dump(prep(foo3()));
  var_dump(prep(foo4()));
}
