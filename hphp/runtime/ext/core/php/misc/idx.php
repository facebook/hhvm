<?hh
<<file: __EnableUnstableFeatures("readonly")>>

namespace HH {

/**
 * Returns the value at a key of a KeyedContainer, if this key is present.
 * This function simplifies the common pattern of checking for a key in
 * a KeyedContainer and using a default value if the key is not present.
 * You should NOT use `idx` as a replacement for accessing elements
 * of KeyedContainers, since this makes the code harder to reason about.
 *
 * `idx` is used to try and index into a KeyedContainer, and return either
 * the value found at this key or some default. Writing this out the
 * long way would look like this:
 *
 * ```
 * C\contains_key($keyedcontainer, 'key') ? $keyedcontainer['key'] : $default;
 * ```
 *
 * This is verbose, and duplicates the variable name and index name, which can
 * lead to errors. With `idx`, you can simplify the expression:
 *
 * ```
 * idx($keyedcontainer, 'key', $default);
 * ```
 *
 * The value `$default` is optional, and defaults to null if unspecified.
 *
 * The first argument is permitted to be null; if it is null,
 * `idx` will always return `$default`.
 *
 * The second argument is permitted to be null; if it is null,
 * `idx` will always return `$default`.
 *
 * Just as an aside, Hack has a null coalesce operator which interacts with
 * with subscripting operations in an unusual way.
 * The important difference between `$dict['key'] ?? $default` and
 * `idx($dict, 'key', $default)` is that the `??` operator will also
 * resolve the `$default` if the value held in `$dict['key']` is null.
 * `idx`, in contrast, will return the null stored at 'key' instead, since 'key' is present.
 *
 * If you notice yourself accessing deeply nested KeyedContainers like this:
 *
 * ```
 * // $dict['key1']['key2']['key3'], but it resolves to null when a key is missing.
 * idx($dict, 'key1', dict[]) |> idx($$, 'key2', dict[]) |> idx($$, 'key3');
 * ```
 *
 * it may be more natural to use `??` instead.
 *
 * ```
 * $dict['key1']['key2']['key3'] ?? null;
 * ```
 *
 * You should NOT use `idx` as a general replacement for accessing KeyedContainer
 * indices. If you expect 'key' to always exist, do not use `idx`!
 *
 * ```
 * // COUNTEREXAMPLE
 * $dict['key'] = some_function();
 * // code...
 * $y = idx($dict, 'key');
 * ```
 *
 * This code is misleading, since the default value of `idx` (null) will never be used.
 * This confuses the reader and leads to annoying nullchecks in the code using `$y`.
 * Since we know that 'key' should / must be present, it is best to stick to
 * indexing with the subscript operator like so.
 *
 * ```
 * $dict['key'];
 * ```
 *
 * This will throw an OutOfBoundsException if the 'key' is somehow not present.
 * This is a good thing, since it will alert you that your mental model
 * of the code is wrong, instead of continuing with `null` (or the default) silently.
 *
 * `idx` is for default selection, not a blanket replacement for array access.
 *
 * If you are tasked with fixing a bug that is caused by an OutOfBoundsException
 * it is often tempting to use `idx` and set a default in place.
 * This is hiding the underlying problem.
 * Chances are that the programmer before you actually expected the key to always be present.
 * Try to figure out why this might be the case.
 * If this KeyedContainer is being used to access keys which are static in the source code,
 * it might be helpful to change the code to use a shape() instead if possible.
 * This will instruct the typechecker to validate that the keys are present.
 *
 * The following behavior is deprecated and should not be relied upon.
 * Because of backwards compatiblity, `idx` treats strings like arrays of characters.
 * This is not allowed by the typechecker, since string is not a KeyedContainer.
 * Indexing into a string can be done safely like so: `$string[4] ?? null`.
 *
 * @param ?KeyedContainer $arr - KeyedContainer to look for an index in.
 * @param ?arraykey $idx      - Index to check for.
 * @param mixed $default      - Default value to return if index is not found. By
 *                              default, this is null.
 * @return mixed                Value at array index if it exists,
 *                              or the default value if not.
 */
function idx(
  mixed $arr,
  mixed $idx,
  mixed $default = null,
)[]: mixed {
  if (\HH\is_any_array($arr)) {
    return \hphp_array_idx($arr, $idx, $default);
  }

  if ($idx !== null) {
    if (\is_object($arr)) {
      // We have to cast to `nothing` here as we're attempting to pass a local
      // to an unbound generic: we don't have access to that specific type.
      $idx = HH\FIXME\UNSAFE_CAST<nonnull, nothing>($idx);
      if ($arr is \ConstIndexAccess<_, _>) {
        if ($arr->containsKey($idx)) {
          return HH\FIXME\UNSAFE_CAST<mixed, KeyedContainer<arraykey, mixed>>(
            $arr,
          )[$idx];
        }
      } else if ($arr is \ConstSet<_>) {
        if ($arr->contains($idx)) {
          return $idx;
        }
      }
    } else if ($arr is string) {
      if (isset($arr[HH\FIXME\UNSAFE_CAST<nonnull, int>($idx)])) {
        return $arr[(int)$idx];
      }
    }
  }

  return $default;
}

<<__IgnoreReadonlyLocalErrors>>
function idx_readonly(
  readonly mixed $arr,
  readonly mixed $idx,
  readonly mixed $default = null,
)[]: readonly mixed {
  return readonly idx(
    $arr,
    $idx,
    HH\FIXME\UNSAFE_CAST<mixed, nothing>($default),
  );
}

}
