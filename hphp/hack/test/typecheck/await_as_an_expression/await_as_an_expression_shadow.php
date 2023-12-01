<?hh

function id<T>(T $x): T {
  return $x;
}

async function gena<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<darray<Tk, Tv>> { throw new Exception("A"); }

async function foo(): Awaitable<darray<string, int>> {
  $gens = dict[
    'start' => async { return 42; }
  ];
  $x = null;
  try {
    $x = id(await gena($gens));
  } catch (Exception $e) {}
  if ($x === null) {
    $x = dict[];
  }
  return $x;
}
