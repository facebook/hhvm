<?hh

namespace HH\Asio {

/**
 * Translate a map of awaitables into a single awaitable of map.
 */
async function m<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, Tv>> {
  $wait_handles = Map {};
  foreach ($awaitables as $index => $awaitable) {
    $wait_handles[$index] = $awaitable->getWaitHandle();
  }
  await AwaitAllWaitHandle::fromMap($wait_handles);
  // TODO: When systemlib supports closures
  // return $wait_handles->map($o ==> $o->result());
  $ret = Map {};
  foreach($wait_handles as $key => $value) {
    $ret[$key] = $value->result();
  }
  return $ret;
}

/**
 * Translate a vector of awaitables into a single awaitable of vector.
 */
async function v<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<Tv>> {
  $wait_handles = Vector {};
  $wait_handles->reserve(count($awaitables));
  foreach ($awaitables as $awaitable) {
    $wait_handles[] = $awaitable->getWaitHandle();
  }
  await AwaitAllWaitHandle::fromVector($wait_handles);
  // TODO: When systemlib supports closures
  // return $wait_handles->map($o ==> $o->result());
  $ret = Vector {};
  foreach($wait_handles as $value) {
    $ret[] = $value->result();
  }
  return $ret;
}

} // namespace HH\Asio
