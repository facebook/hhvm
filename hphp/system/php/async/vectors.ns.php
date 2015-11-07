<?hh // strict

namespace HH\Asio {

///// Mapped /////

/**
 * Similar to Vector::map, but maps the values using awaitables
 */
async function vm<Tv, Tr>(
  Traversable<Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<Tr>> {
  $awaitables = Vector { };
  foreach ($inputs as $input) {
    $awaitables[] = $callable($input);
  }
  return await v($awaitables);
}

/**
 * Similar to vm(), but passes element keys as well
 */
async function vmk<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<Tr>> {
  $awaitables = Vector { };
  foreach ($inputs as $k => $v) {
    $awaitables[] = $callable($k, $v);
  }
  return await v($awaitables);
}

///// Filtered /////

/**
 * Apply an async filtering function, and return a Vector of outputs.
 */
async function vf<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Vector<T>> {
  $tests = await mm($inputs, $callable);
  $results = Vector {};
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      $results[] = $value;
    }
  }
  return $results;
}

/**
 * Similar to vf(), but passes element keys as well
 */
async function vfk<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (Tk, T): Awaitable<bool>) $callable,
): Awaitable<Vector<T>> {
  $tests = await mmk($inputs, $callable);
  $results = Vector {};
  foreach ($inputs as $key => $value) {
    if ($tests[$key]) {
      $results[] = $value;
    }
  }
  return $results;
}

////////////////////
////// Wrapped /////
////////////////////

/**
 * Same as v(), but wrap results into ResultOrExceptionWrappers.
 */
async function vw<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<ResultOrExceptionWrapper<Tv>>> {
  $wrapped = Vector { };
  foreach ($awaitables as $input) {
    $wrapped[] = wrap($input);
  }
  return await v($wrapped);
}

///// Mapped /////

/**
 * Like vm(), except using a ResultOrExceptionWrapper.
 */
async function vmw<Tv, Tr>(
  Traversable<Tv> $inputs,
  (function (Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<Tr>>> {
  $mapped = Vector {};
  foreach ($inputs as $input) {
    $mapped[] = wrap($callable($input));
  }
  return await v($mapped);
}

/**
 * Like vmk(), except using a ResultOrExceptionWrapper.
 */
async function vmkw<Tk, Tv, Tr>(
  KeyedTraversable<Tk, Tv> $inputs,
  (function (Tk, Tv): Awaitable<Tr>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<Tr>>> {
  $mapped = Vector { };
  foreach ($inputs as $k => $v) {
    $mapped[] = wrap($callable($k, $v));
  }
  return await v($mapped);
}

///// Filtered /////

/**
 * Like vf(), except using a ResultOrExceptionWrapper.
 */
async function vfw<Tk,T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>> {
  $mapped = Map { };
  foreach ($inputs as $k => $v) {
    $mapped[$k] = wrap($callable($v));
  }
  $tests = await m($mapped);
  $results = Vector {};
  foreach ($inputs as $key => $value) {
    $test = $tests[$key];
    if ($test->isFailed()) {
      $results[] = new WrappedException($test->getException());
    } else if ($test->getResult() === true) {
      $results[] = new WrappedResult($value);
    }
  }
  return $results;
}

/**
 * Like vfk(), except using a ResultOrExceptionWrapper.
 */
async function vfkw<Tk, T>(
  KeyedTraversable<Tk, T> $inputs,
  (function (Tk, T): Awaitable<bool>) $callable,
): Awaitable<Vector<ResultOrExceptionWrapper<T>>> {
  $mapped = Map { };
  foreach ($inputs as $k => $v) {
    $mapped[$k] = wrap($callable($k, $v));
  }
  $tests = await m($mapped);

  $results = Vector {};
  foreach ($inputs as $key => $value) {
    $test = $tests[$key];
    if ($test->isFailed()) {
      $results[] = new WrappedException($test->getException());
    } else if ($test->getResult() === true) {
      $results[] = new WrappedResult($value);
    }
  }
  return $results;
}

} // namespace HH\Asio
