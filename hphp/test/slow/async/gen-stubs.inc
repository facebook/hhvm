<?hh

async function genva(...$args): Awaitable/*<(...)>*/ {
  $awaitables = varray[];
  foreach ($args as $value) {
    if ($value === null) {
      $awaitables[] = async { return null; };
    } else {
      $awaitables[] = $value;
    }
  }
  unset($args);
  await AwaitAllWaitHandle::fromArray($awaitables);
  foreach ($awaitables as $index => $value) {
    $awaitables[$index] = \HH\Asio\result($value);
  }
  return $awaitables;
}

async function gena<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
  // necessary for checking number of arguments passed to gena
  // otherwise hhbbc discards extra arguments
  ...$unused
): Awaitable<darray<Tk, Tv>> {
  if (func_num_args() != 1) {
    trigger_error("Call to undefined function gena()", E_USER_ERROR);
  }
  $awaitables = darray($awaitables);
  await AwaitAllWaitHandle::fromDArray($awaitables);
  foreach ($awaitables as $index => $value) {
    $awaitables[$index] = \HH\Asio\result($value);
  }
  return $awaitables;
}
