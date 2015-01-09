<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/**
 * API Overview
 *
 * id() - Take an Awaitable and return it unmodified
 * wrap() - Await an Awaitable and wrap it in a ResultOrExceptionWrapper
 * call() - Call a function which returns an awaitable, and return that
 * val() - A static value, yielded from an Awaitable immediately
 *
 * later() - Empty Awaitable which will schedule at lower priority
 * usleep() - Empty Awaitable which will yield in $usecs microseconds
 *
 * The remaining methods are named according to a matrix of their attributes:
 *
 * First, how they take and return arguments according to types:
 *   a - Array
 *   v - Vector
 *   m - Map
 *   va - Variadic args, Array return value
 *
 * Next, methods which use a factory function to produce Awaitables
 *   are denoted with a 'c'
 *
 * Next, methods which wrap their results in a ResultOrExceptionWrapper
 *   are denoted with a 'w'
 *
 * This yields 16 methods:
 *   a(),  ac(),  aw(),  acw(),
 *   v(),  vc(),  vw(),  vcw(),
 *   m(),  mc(),  mw(),  mcw(),
 *   va(), vac(), vaw(), vacw(),
 *
 * Separately from the above matrix, There exists a set of filter/map methods
 *   for each container type which focus on the test function itself being
 *   an awaitable.
 *
 * First comes the container type, which excludes Variadics
 *   'a' - Array
 *   'v' - Vector
 *   'm' - Map
 *
 * Next, the operation type:
 *   'f' - Filter
 *   'm' - Map
 *
 * Finally, an optional 'k' modifier to indicate callback takes a key as well
 *
 * This yields 12 methods:
 *   af(), afk(), am(), amk(),
 *   vf(), vfk(), vm(), vmk(),
 *   mf(), mfk(), mm(), mmk(),
 *
 * Callbacks to these methods are of the form:
 *   *f()  - (function <Tv>(Tv): Awaitable<bool>)
 *   *fk() - (function <Tk, Tv>(Tk, Tv): Awaitable<bool>)
 *   *m()  - (function <Tv, Tr>(Tv): Awaitable<Tr>)
 *   *mk() - (function <Tk, Tv, Tr>(Tk, Tv): Awaitable<Tr>)
 */

namespace HH\Asio {

//
// Identities - Don't really do much, but exist for completeness
//

/**
 * Translate an awaitable into itself.
 */
async function id<Tv>(Awaitable<Tv> $awaitable): Awaitable<Tv> {
  return await $awaitable;
}

/**
 * Same as id(), but wrap result into ResultOrExceptionWrapper.
 */
async function wrap<Tv>(
  Awaitable<Tv> $awaitable,
): Awaitable<ResultOrExceptionWrapper<Tv>> {
  try {
    $result = await $awaitable;
    return new WrappedResult($result);
  } catch (Exception $e) {
    return new WrappedException($e);
  }
}

/**
 * Invoke and then await. Useful only idiomatically for invoking async
 * lambdas
 *
 *  await va(
 *    gen_something(),
 *    call(async () ==> await something_else()),
 *  );
 */
async function call<Tv>(
  (function (): Awaitable<Tv>) $gen,
): Awaitable<Tv> {
  return await $gen();
}

/**
 * Await on a known value
 */
async function val<T>(T $v): Awaitable<T> {
  return $v;
}

/**
 * Wait until some later time in the future.
 */
async function later(): Awaitable<void> {
  // reschedule to the lowest priority
  return await RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    0,
  );
}

/**
 * Convenience wrapper for SleepWaitHandle
 */
async function usleep(
  int $usecs,
): Awaitable<void> {
  return await SleepWaitHandle::create($usecs);
}

//
// Array
//

/**
 * Translate an array of Awaitables into a single Awaitable<array>.
 */
async function a<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables
): Awaitable<array<Tk, Tv>> {
  $wait_handles = array();
  foreach ($awaitables as $index => $awaitable) {
    $wait_handles[$index] = $awaitable->getWaitHandle();
  }

  await AwaitAllWaitHandle::fromArray($wait_handles);
  // TODO: When systemlib supports closures:
  // return array_map($o ==> $o->result(), $wait_handles);
  $ret = array();
  foreach($wait_handles as $key => $value) {
    $ret[$key] = $value->result();
  }
  return $ret;
}

/**
 * Same as a(), but wrap results into ResultOrExceptionWrappers.
 */
async function aw<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<array<Tk, ResultOrExceptionWrapper<Tv>>> {
  $wait_handles = array();
  foreach ($awaitables as $index => $awaitable) {
    $wait_handles[$index] = wrap($awaitable)->getWaitHandle();
  }
  await AwaitAllWaitHandle::fromArray($wait_handles);
  // TODO: When systemlib supports closures
  // return array_map($o ==> $o->result(), $wait_handles);
  $ret = array();
  foreach ($wait_handles as $key => $value) {
    $ret[$key] = $value->result();
  }
  return $ret;
}

/**
 * Yield an array of values indexed by key for Awaitables
 *   produced by a factory function.
 */
async function ac<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<array<Tk, Tv>> {
  $gens = array();
  foreach ($keys as $key) {
    $gens[$key] = $gen($key)->getWaitHandle();
  }
  await AwaitAllWaitHandle::fromArray($gens);
  // TODO: When systemlib supports closures
  // return array_map($o ==> $o->result(), $gens);
  $ret = array();
  foreach($gens as $key => $value) {
    $ret[$key] = $value->result();
  }
  return $ret;
}

/**
 * Same as ac(), but wrap results into ResultOrExceptionWrappers.
 */
async function acw<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<array<Tk, ResultOrExceptionWrapper<Tv>>> {
  $gens = array();
  foreach ($keys as $key) {
    $gens[$key] = wrap($gen($key))->getWaitHandle();
  }
  await AwaitAllWaitHandle::fromArray($gens);
  // TODO: When systemlib supports closures
  // return array_map($o ==> $o->result(), $gens);
  $ret = array();
  foreach($gens as $key => $value) {
    $ret[$key] = $value->result();
  }
  return $ret;
}

/**
 * Like array_filter, but the test function is async.
 *
 * Example:
 *
 *   $things = array(...);
 *   $valid_things = await af(
 *     $things,
 *     async $thing ==> await $thing->genIsValid(),
 *   );
 */
async function af<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<bool>) $callable,
): Awaitable<array<Tk, Tv>> {
  $gens = array();
  foreach ($inputs as $key => $value) {
    $gens[$key] = $callable($value);
  }
  $tests = await a($gens);
  $results = array();
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      // array_filter preserves keys, so we do the same.
      $results[$key] = $value;
    }
  }
  return $results;
}

/**
 * Similar to af(), but passes the element's key as well
 */
async function afk<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<bool>) $callable,
): Awaitable<array<Tk, Tv>> {
  $gens = array();
  foreach ($inputs as $key => $value) {
    $gens[$key] = $callable($key, $value);
  }
  $tests = await a($gens);
  $results = array();
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      // array_filter preserves keys, so we do the same.
      $results[$key] = $value;
    }
  }
  return $results;
}

/**
 * Similar to array_map, but maps the values using awaitables
 */
async function am<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tv>) $callable,
): Awaitable<array<Tk, Tv>> {
  $gens = array();
  foreach ($inputs as $key => $value) {
    $gens[$key] = $callable($value);
  }
  return await a($gens);
}

/**
 * Similar to am(), but passes element keys as well
 */
async function amk<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tv>) $callable,
): Awaitable<array<Tk, Tv>> {
  $gens = array();
  foreach ($inputs as $key => $value) {
    $gens[$key] = $callable($key, $value);
  }
  return await a($gens);
}

//
// Maps
//

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
 * Same as m(), but wrap results into ResultOrExceptionWrappers.
 */
async function mw<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tv>>> {
  $wait_handles = Map {};
  foreach ($awaitables as $index => $awaitable) {
    $wait_handles[$index] = wrap($awaitable)->getWaitHandle();
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
 * Yield a map of values indexed by key for a given vector of keys.
 */
async function mc<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<Map<Tk, Tv>> {
  $gens = Map {};
  foreach ($keys as $key) {
    $gens[$key] = $gen($key)->getWaitHandle();
  }
  await AwaitAllWaitHandle::fromMap($gens);
  // TODO: When systemlib supports closures
  // return $gens->map($o ==> $o->result());
  $ret = Map {};
  foreach($gens as $key => $value) {
    $ret[$key] = $value->result();
  }
  return $ret;
}


/**
 * Same as mc(), but wrap results into ResultOrExceptionWrappers.
 */
async function mcw<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tv>>> {
  $gens = Map {};
  foreach ($keys as $key) {
    $gens[$key] = wrap($gen($key))->getWaitHandle();
  }
  await AwaitAllWaitHandle::fromMap($gens);
  // TODO: When systemlib supports closures
  // return $gens->map($o ==> $o->result());
  $ret = Map {};
  foreach($gens as $key => $value) {
    $ret[$key] = $value->result();
  }
  return $ret;
}

/**
 * Map version of af()
 */
async function mf<Tk, Tv>(
  \ConstMap<Tk, Tv> $inputs,
  (function (Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {
  $gens = $inputs->map($callable);
  $tests = await m($gens);
  $results = Map {};
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      // array_filter preserves keys, so we do the same.
      $results[$key] = $value;
    }
  }
  return $results;
}

/**
 * Similar to mfk(), but passes element keys as well
 */
async function mfk<Tk, Tv>(
  \ConstMap<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {
  $gens = $inputs->mapWithKey($callable);
  $tests = await m($gens);
  $results = Map {};
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      // array_filter preserves keys, so we do the same.
      $results[$key] = $value;
    }
  }
  return $results;
}

/**
 * Similar to Map::map, but maps the values using awaitables
 */
async function mm<Tk, Tv>(
  \ConstMap<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tv>) $callable,
): Awaitable<Map<Tk, Tv>> {
  return await m($inputs->map($callable));
}

/**
 * Similar to mm(), but passes element keys as well
 */
async function mmk<Tk, Tv>(
  \ConstMap<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tv>) $callable,
): Awaitable<Map<Tk, Tv>> {
  return await m($inputs->mapWithKey($callable));
}

//
// Vectors
//

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

/**
 * Same as v(), but wrap results into ResultOrExceptionWrappers.
 */
async function vw<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<ResultOrExceptionWrapper<Tv>>> {
  $wait_handles = Vector {};
  $wait_handles->reserve(count($awaitables));
  foreach ($awaitables as $awaitable) {
    $wait_handles[] = wrap($awaitable)->getWaitHandle();
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

/**
 * Yield the vector of values created by
 * mapping each element of $keys through $gen and awaiting the results.
 */
async function vc<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<Vector<Tv>> {
  $wait_handles = Vector {};
  $wait_handles->reserve(count($keys));
  foreach ($keys as $key) {
    $wait_handles[] = $gen($key)->getWaitHandle();
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

/**
 * Same as vc(), but wrap results into ResultOrExceptionWrappers.
 */
async function vcw<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<Vector<ResultOrExceptionWrapper<Tv>>> {
  $wait_handles = Vector {};
  $wait_handles->reserve(count($keys));
  foreach ($keys as $key) {
    $wait_handles[] = wrap($gen($key))->getWaitHandle();
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

/**
 * Vector version of af()
 */
async function vf<T>(
  \ConstVector<T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Vector<T>> {
  $gens = $inputs->map($callable);
  $tests = await v($gens);
  $results = Vector {};
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      // array_filter preserves keys, so we do the same.
      $results[] = $value;
    }
  }
  return $results;
}

/**
 * Similar to vf(), but passes element keys as well
 */
async function vfk<T>(
  \ConstVector<T> $inputs,
  (function (int, T): Awaitable<bool>) $callable,
): Awaitable<Vector<T>> {
  $gens = $inputs->mapWithKey($callable);
  $tests = await v($gens);
  $results = Vector {};
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      // array_filter preserves keys, so we do the same.
      $results[] = $value;
    }
  }
  return $results;
}

/**
 * Similar to Vector::map, but maps the values using awaitables
 */
async function vm<T>(
  \ConstVector<T> $inputs,
  (function (T): Awaitable<T>) $callable,
): Awaitable<Vector<T>> {
  return await v($inputs->map($callable));
}

/**
 * Similar to vm(), but passes element keys as well
 */
async function vmk<T>(
  \ConstVector<T> $inputs,
  (function (int, T): Awaitable<T>) $callable,
): Awaitable<Vector<T>> {
  return await v($inputs->mapWithKey($callable));
}

//
// Variadics
//

/**
 * Translate a varargs of Awaitables into a single Awaitable<array>.
 */
async function va<T>(...$args): Awaitable<array<T>> {
  return await a($args);
}

/**
 * Same as va(), but wrap results into ResultOrExceptionWrappers.
 */
async function vaw<T>(...$args): Awaitable<array<ResultOrExceptionWrapper<T>>> {
  return await aw($args);
}

/**
 * Yield an array of values indexed by key for Awaitables
 *   produced by a factory function.
 */
async function vac<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  ...$keys
): Awaitable<array<Tk, Tv>> {
  return await ac($gen, $keys);
}

/**
 * Same as vac(), but wrap results into ResultOrExceptionWrappers.
 */
async function vacw<Tk, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  ...$keys
): Awaitable<array<Tk, ResultOrExceptionWrapper<Tv>>> {
  return await acw($gen, $keys);
}

} // namespace HH\Asio
