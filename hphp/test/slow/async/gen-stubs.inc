<?hh
async function gen_null<T>(): Awaitable<?T> {
  return null;
}

async function genva(...$args): Awaitable/*<(...)>*/ {
  $wait_handles = varray[];
  foreach ($args as $value) {
    if ($value === null) {
      $wait_handles[] = gen_null()->getWaitHandle();
    } else {
      $wait_handles[] = $value instanceof WaitHandle
        ? $value
        : $value->getWaitHandle();
    }
  }
  unset($args);
  await AwaitAllWaitHandle::fromArray($wait_handles);
  foreach ($wait_handles as $index => $value) {
    $wait_handles[$index] = \HH\Asio\result($value);
  }
  return $wait_handles;
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
  $wait_handles = darray[];
  foreach ($awaitables as $index => $value) {
    $wait_handles[$index] = $value instanceof WaitHandle
      ? $value
      : $value->getWaitHandle();
  }
  unset($awaitables);
  await @AwaitAllWaitHandle::fromArray($wait_handles);
  foreach ($wait_handles as $index => $value) {
    $wait_handles[$index] = \HH\Asio\result($value);
  }
  return $wait_handles;
}
