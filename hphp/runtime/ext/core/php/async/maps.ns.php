<?hh

namespace HH\Asio {

///// Mapped /////

/**
 * Returns an `Awaitable` of `Map` containing after a mapping operation has
 * been applied to each value in the provided `KeyedTraversable`.
 *
 * This function is similar to `Map::map()`, but the mapping of the values
 * is done using `Awaitable`s.
 *
 * This function is called `mm` because we are returning a `m`ap, and doing a
 * `m`apping operation.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The keys and values in the `Map` of the returned `Awaitable` are not
 * available until you `await` or `join` the returned `Awaitable`.
 *
 * @deprecated Use `Dict\map_async()` instead.
 *
 * @param $inputs - The `KeyedTraversable` of values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` after the mapping operation has been
 *           applied to the values in  `$inputs`.
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
 * Returns an `Awaitable` of `Map` after a mapping operation has been
 * applied to each key and value in the provided `KeyedTraversable`.
 *
 * This function is similar to `mm()`, but passes element keys to the callable
 * as well.
 *
 * This function is similar to `Map::mapWithKey()`, but the mapping of the keys
 * and values is done using `Awaitable`s.
 *
 * This function is called `mmk` because we are returning a `m`ap and doing a
 * a `m`apping operation that includes `k`eys.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The keys and values in the `Map` of the returned `Awaitable` are not
 * available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` after the mapping operation has been
 *           applied to both the keys and values in `$inputs`.
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
 * Returns an `Awaitable` of `Map` after a filtering operation has been
 * applied to each value in the provided `KeyedTraversable`.
 *
 * This function is similar to `Map::filter()`, but the filtering of the
 * values is done using `Awaitable`s.
 *
 * This function is called `mf` because we are returning a `m`ap, and we are
 * doing a `f`iltering operation.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The keys and values in the `Map` of the returned `Awaitable` are not
 * available until you `await` or `join` the returned `Awaitable`.
 *
 * @deprecated Use `Dict\filter_async()` instead.
 *
 * @param $inputs - The `KeyedTraversable` of values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` after the filtering operation has been
 *           applied to the values in  `$inputs`.
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
 * Returns an `Awaitable` of `Map` after a filtering operation has been
 * applied to each key and value in the provided `KeyedTraversable`.
 *
 * This function is similar to `mf()`, but passes element keys to the callable
 * as well.
 *
 * This function is similar to `Map::filterWithKey()`, but the filtering of the
 * keys and values is done using `Awaitable`s.
 *
 * This function is called `mfk` because we are returning a `m`ap, doing a
 * a `f`iltering operation that includes `k`eys.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The keys and values in the `Map` of the returned `Awaitable` are not
 * available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to filter.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` after the filtering operation has been
 *           applied to both the keys and values in `$inputs`.
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
 * Translate a `KeyedTraversable` of `Awaitables` into a single `Awaitable` of
 * `Map` of key/`ResultOrExceptionWrapper` pairs.
 *
 * This function is the same as `m()`, but wraps the results into
 * key/`ResultOrExceptionWrapper` pairs.
 *
 * This function takes any `KeyedTraversable` object of `Awaitables` (i.e., each
 * member of the `KeyedTraversable` has a value of type `Awaitable`, likely
 * from a call to a function that returned `Awaitable<T>`), and transforms those
 * `Awaitables` into one big `Awaitable` `Map` of
 * key/`ResultOrExceptionWrapper` pairs.
 *
 * This function is called `mw` because we are returning a `m`ap of
 * `Awaitable` `w`rapped into `ResultofExceptionWrapper`s.
 *
 * The `ResultOrExceptionWrapper` values in the `Map` of the returned
 * `Awaitable` are not available until you `await` or `join` the returned
 * `Awaitable`.
 *
 * @param $awaitables - The collection of `KeyedTraversable` awaitables.
 *
 * @return - An `Awaitable` of `Map` of key/`ResultOrExceptionWrapper` pairs,
 *           where the `Map` was generated from each `KeyedTraversable` member
 *           in `$awaitables`.
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
 * Returns an `Awaitable` of `Map` of `ResultOrExceptionWrapper` after a
 * mapping operation has been applied to each value in the provided
 * `KeyedTraversable`.
 *
 * This function is similar to `mm()`, except the `Map` in the returned
 * `Awaitable` contains values of `ResultOrExceptionWrapper` instead of raw
 * values.
 *
 * This function is similar to `Map::map()`, but the mapping of the values
 * is done using `Awaitable`s.
 *
 * This function is called `mmw` because we are returning a `m`ap, doing a
 * `m`apping operation and each value member in the `Map` is `w`rapped by a
 * `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The `ResultOrExceptionWrapper`s in the `Map` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` of key/`ResultOrExceptionWrapper` pairs
 *           after the mapping operation has been applied to the values in
 *           `$inputs`.
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
 * Returns an `Awaitable` of `Map` of `ResultOrExceptionWrapper` after a
 * mapping operation has been applied to each key/value pair in the provided
 * `KeyedTraversable`.
 *
 * This function is similar to `mmk()`, except the `Map` in the returned
 * `Awaitable` contains values of `ResultOrExceptionWrapper` instead of raw
 * values.
 *
 * This function is similar to `Map::mapWithKey()`, but the mapping of the keys
 * and values is done using `Awaitable`s.
 *
 * This function is called `mmkw` because we are returning a `m`ap, doing a
 * `m`apping operation on `k`eys and values, and each value member in the `Map`
 * is `w`rapped by a `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable`.
 *
 * The `ResultOrExceptionWrapper`s in the `Map` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to map.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` of key/`ResultOrExceptionWrapper` pairs
 *           after the mapping operation has been applied to the keys an values
 *           in `$inputs`.
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
 * Returns an `Awaitable` of `Map` of `ResultOrExceptionWrapper` after a
 * filtering operation has been applied to each value in the provided
 * `KeyedTraversable`.
 *
 * This function is similar to `mf()`, except the `Map` in the returned
 * `Awaitable` contains values of `ResultOrExceptionWrapper` instead of raw
 * values.
 *
 * This function is similar to `Map::filter()`, but the filtering of the values
 * is done using `Awaitable`s.
 *
 * This function is called `mfw` because we are returning a `m`ap, doing a
 * `f`iltering operation and each value member in the `Map` is `w`rapped by a
 * `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The `ResultOrExceptionWrapper`s in the `Map` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of values to fitler.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` of key/`ResultOrExceptionWrapper` pairs
 *           after the filterin operation has been applied to the values in
 *           `$inputs`.
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
 * Returns an `Awaitable` of `Map` of `ResultOrExceptionWrapper` after a
 * filtering operation has been applied to each key/value pair in the provided
 * `KeyedTraversable`.
 *
 * This function is similar to `mfk()`, except the `Map` in the returned
 * `Awaitable` contains values of `ResultOrExceptionWrapper` instead of raw
 * values.
 *
 * This function is similar to `Map::filterWithKey()`, but the filtering of the
 * keys and values is done using `Awaitable`s.
 *
 * This function is called `mfkw` because we are returning a `m`ap, doing a
 * `f`iltering operation on `k`eys and values, and each value member in the
 * `Map` is `w`rapped by a `ResultOrExceptionWrapper`.
 *
 * `$callable` must return an `Awaitable` of `bool`.
 *
 * The `ResultOrExceptionWrapper`s in the `Map` of the returned `Awaitable`
 * are not available until you `await` or `join` the returned `Awaitable`.
 *
 * @param $inputs - The `KeyedTraversable` of keys and values to filter.
 *
 * @param $callable - The callable containing the `Awaitable` operation to
 *                    apply to `$inputs`.
 *
 * @return - An `Awaitable` of `Map` of key/`ResultOrExceptionWrapper` pairs
 *           after the filtering operation has been applied to the keys an
 *           values in `$inputs`.
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
