<?hh

namespace HH\Asio {

///// Mapped /////

/**
 * Returns an `Awaitable` of `Vector` containing after a mapping operation has
 * been applied to each value in the provided `Traversable`.
 *
 * This function is similar to `Vector::map()`, but the mapping of the values
 * is done using `Awaitable`s.
 *
 * This function is called `vm` because we are returning a `v`ector, and
 * we are doing a `m`apping operation.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The values in the `Vector` of the returned `Awaitable` are not available
 * until you `await` or `join` the returned `Awaitable`.
 *
 * @deprecated Use `Vec\map_async()` instead.
 *
 * @param $inputs - The `Traversable` of values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` after the mapping operation has been
 *           applied to the values in  `$inputs`.
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
 * Returns an `Awaitable` of `Vector` after a mapping operation has been
 * applied to each key and value in the provided `KeyedTraversable`.
 *
 * This function is similar to `vm()`, but passes element keys to the callable
 * as well.
 *
 * This function is similar to `Vector::mapWithKey()`, but the mapping of the
 * keys and values is done using `Awaitable`s.
 *
 * This function is called `vmk` because we are returning a `v`ector and doing
 * a `m`apping operation that includes `k`eys.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The values in the `Vector` of the returned `Awaitable` are not available
 * until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` after the mapping operation has been
 *           applied to both the keys and values in `$inputs`.
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
 * Returns an `Awaitable` of `Vector` after a filtering operation has been
 * applied to each value in the provided `KeyedTraversable`.
 *
 * This function is similar to `Vector::filter()`, but the filtering of the
 * values is done using `Awaitable`s.
 *
 * This function is called `vf` because we are returning a `v`ector, and
 * we are doing a `f`iltering operation.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The values in the `Vector` of the returned `Awaitable` are not available
 * until you `await` or `join` the returned `Awaitable`.
 *
 * @deprecated Use `Vec\filter_async()` instead.
 *
 * @param $inputs - The `KeyedTraversable` of values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` after the filtering operation has been
 *           applied to the values in  `$inputs`.
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
 * Returns an `Awaitable` of `Vector` after a filtering operation has been
 * applied to each key and value in the provided `KeyedTraversable`.
 *
 * This function is similar to `vf()`, but passes element keys to the callable
 * as well.
 *
 * This function is similar to `Vector::filterWithKey()`, but the filtering of
 * the keys and values is done using `Awaitable`s.
 *
 * This function is called `vfk` because we are returning a `v`ector, doing a
 * a `f`iltering operation that includes `k`eys.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The values in the `Vector` of the returned `Awaitable` are not available
 * until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to filter.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` after the filtering operation has been
 *           applied to both the keys and values in `$inputs`.
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
 * Translate a `Traversable` of `Awaitables` into a single `Awaitable` of
 * `Vector` of `ResultOrExceptionWrapper`.
 *
 * This function is the same as `v()`, but wraps the results into
 * `ResultOrExceptionWrapper`s.
 *
 * This function takes any `Traversable` object of `Awaitables` (i.e., each
 * member of the `Traversable` is of type of `Awaitable`, likely from a call
 * to a function that returned `Awaitable<T>`), and transforms those
 * `Awaitables` into one big `Awaitable` `Vector` of `ResultOrExceptionWrapper`.
 *
 * This function is called `vw` because we are returning a `v`ector of
 * `Awaitable` `w`rapped into `ResultofExceptionWrapper`s.
 *
 * The `ResultOrExceptionWrapper`s in the `Vector` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $awaitables - The collection of `Traversable` awaitables.
 *
 * @return - An `Awaitable` of `Vector` of `ResultOrExceptionWrapper`, where
 *           the `Vector` was generated from each `Traversable` member in
 *           `$awaitables`.
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
 * Returns an `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after a
 * mapping operation has been applied to each value in the provided
 * `Traversable`.
 *
 * This function is similar to `vm()`, except the `Vector` in the returned
 * `Awaitable` contains `ResultOrExceptionWrapper`s instead of raw values.
 *
 * This function is similar to `Vector::map()`, but the mapping of the values
 * is done using `Awaitable`s.
 *
 * This function is called `vmw` because we are returning a `v`ector, doing a
 * `m`apping operation and each member of the `Vector` is `w`rapped by a
 * `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The `ResultOrExceptionWrapper`s in the `Vector` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `Traversable` of values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after the
 *           mapping operation has been applied to the values in `$inputs`.
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
 * Returns an `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after a
 * mapping operation has been applied to each key/value pair in the provided
 * `KeyedTraversable`.
 *
 * This function is similar to `vmk()`, except the `Vector` in the returned
 * `Awaitable` contains `ResultOrExceptionWrapper`s instead of raw values.
 *
 * This function is similar to `Vector::mapWithKey()`, but the mapping of the
 * key/value pairs are done using `Awaitable`s.
 *
 * This function is called `vmkw` because we are returning a `v`ector, doing a
 * `m`apping operation that includes both `k`eys and values, and each member
 * of the `Vector` is `w`rapped by a `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The `ResultOrExceptionWrapper`s in the `Vector` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after the
 *           mapping operation has been applied to the keys and values in
 *           `$inputs`.
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
 * Returns an `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after a
 * filtering operation has been applied to each value in the provided
 * `KeyedTraversable`.
 *
 * This function is similar to `vf()`, except the `Vector` in the returned
 * `Awaitable` contains `ResultOrExceptionWrapper`s instead of raw values.
 *
 * This function is similar to `Vector::filter()`, but the mapping of the values
 * is done using `Awaitable`s.
 *
 * This function is called `vfw` because we are returning a `v`ector, doing a
 * `f`iltering operation and each member of the `Vector` is `w`rapped by a
 * `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The `ResultOrExceptionWrapper`s in the `Vector` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after the
 *           filtering operation has been applied to the values in `$inputs`.
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
 * Returns an `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after a
 * filtering operation has been applied to each key/value pair in the provided
 * `KeyedTraversable`.
 *
 * This function is similar to `vfk()`, except the `Vector` in the returned
 * `Awaitable` contains `ResultOrExceptionWrapper`s instead of raw values.
 *
 * This function is similar to `Vector::filterWithKey()`, but the mapping of the
 * key/value pairs are done using `Awaitable`s.
 *
 * This function is called `vfkw` because we are returning a `v`ector, doing a
 * `f`iltering operation that includes both `k`eys and values, and each member
 * of the `Vector` is `w`rapped by a `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The `ResultOrExceptionWrapper`s in the `Vector` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Vector` of `ResultOrExceptionWrapper` after the
 *           filtering operation has been applied to the keys and values in
 *           `$inputs`.
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
