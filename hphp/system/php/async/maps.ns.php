<?hh // strict

namespace HH\Asio {

///// Mapped /////

/**
 * Similar to Map::map, but maps the values using awaitables
 */
async function mm<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>> {
  $awaitables = Map { };
  foreach ($inputs as $k => $v) {
    $awaitables[$k] = $callable($v);
  }
  return await m($awaitables);
}

/**
 * Similar to mm(), but passes element keys as well
 */
async function mmk<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, Tr>> {
  $awaitables = Map { };
  foreach ($inputs as $k => $v) {
    $awaitables[$k] = $callable($k, $v);
  }
  return await m($awaitables);
}

///// Filtered /////

/**
 * Filter a Map with an Awaitable callback
 */
async function mf<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {
  $tests = await mm($inputs, $callable);
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
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, Tv>> {
  $tests = await mmk($inputs, $callable);
  $results = Map {};
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      // array_filter preserves keys, so we do the same.
      $results[$key] = $value;
    }
  }
  return $results;
}

////////////////////
////// Wrapped /////
////////////////////

/**
 * Same as m(), but wrap results into ResultOrExceptionWrappers.
 */
async function mw<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tv>>> {
  $wrapped = Map { };
  foreach ($awaitables as $k => $a) {
    $wrapped[$k] = wrap($a);
  }
  return await m($wrapped);
}

///// Mapped /////

/**
 * Like mm(), except using a ResultOrExceptionWrapper.
 */
async function mmw<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>> {
  $wrapped = Map { };
  foreach ($inputs as $k => $input) {
    $wrapped[$k] = wrap($callable($input));
  }
  return await m($wrapped);
}

/**
 * Like mmk(), except using a ResultOrExceptionWrapper.
 */
async function mmkw<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<Tr>>> {
  $wrapped = Map { };
  foreach ($inputs as $k => $input) {
    $wrapped[$k] = wrap($callable($k, $input));
  }
  return await m($wrapped);
}

///// Filtered /////

/**
 * Like mf(), except using a ResultOrExceptionWrapper.
 */
async function mfw<Tk,T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<T>>> {
  $handles = Map { };
  foreach ($inputs as $k => $input) {
    $handles[$k] = wrap($callable($input));
  }
  $tests = await m($handles);

  $results = Map {};
  foreach ($inputs as $key => $value) {
    $test = $tests[$key];
    if ($test->isFailed()) {
      $results[$key] = new WrappedException($test->getException());
    } else if ($test->getResult() === true) {
      $results[$key] = new WrappedResult($value);
    }
  }
  return $results;
}

/**
 * Like mfk(), except using a ResultOrExceptionWrapper.
 */
async function mfkw<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (Tk, T): Awaitable<bool>) $callable,
): Awaitable<Map<Tk, ResultOrExceptionWrapper<T>>> {
  $handles = Map { };
  foreach ($inputs as $k => $v) {
    $handles[$k] = wrap($callable($k, $v));
  }
  $tests = await m($handles);

  $results = Map {};
  foreach ($inputs as $key => $value) {
    $test = $tests[$key];
    if ($test->isFailed()) {
      $results[$key] = new WrappedException($test->getException());
    } else if ($test->getResult() === true) {
      $results[$key] = new WrappedResult($value);
    }
  }
  return $results;
}

} // namespace HH\Asio
