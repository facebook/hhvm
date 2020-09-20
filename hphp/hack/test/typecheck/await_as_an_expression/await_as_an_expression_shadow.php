<?hh // strict

function id<T>(T $x): T {
  return $x;
}

async function foo(): Awaitable<darray<string, int>> {
  $gens = darray[
    'start' => async { return 42; }
  ];
  $x = null;
  try {
    $x = id(await gena($gens));
  } catch (Exception $e) {}
  if ($x === null) {
    $x = darray[];
  }
  return $x;
}
