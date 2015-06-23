<?php

namespace HH {

/**
 * Returns the value at an index of an array. This function simplifies the
 * common pattern of checking for an index in an array and selecting a default
 * value if it does not exist. You should NOT use idx() as a general replacement
 * for accessing array indices.
 *
 * idx() is used to look for an index in an array, and return either the value
 * at that index (if it exists) or some default (if it does not). Without
 * idx(), you need to do this:
 *
 *   array_key_exists('index', $arr) ? $arr['index'] : $default
 *
 * This is verbose, and duplicates the variable name and index name, which can
 * lead to errors. With idx(), you can simplify the expression:
 *
 *   idx($arr, 'index', $default);
 *
 * The value $default is optional, and defaults to null if unspecified.
 *
 * The array $arr is permitted to be null; if it is null, idx() guarantees
 * it will return $default.
 *
 * You should NOT use idx() as a general replacement for accessing array
 * indices. If you expect 'index' to always exist, DON'T use idx()!
 *
 *   COUNTEREXAMPLE
 *   idx($arr, 'index'); // If you expect 'index' to exist, this is WRONG!
 *
 * Instead, just access it normally like a sensible human being:
 *
 *   $arr['index']
 *
 * This will give you a helpful warning if the index doesn't exist, allowing
 * you to identify a bug in your program and fix it. In contrast, idx() will
 * fail silently if the index doesn't exist, which won't help you out at all.
 *
 * idx() is for default selection, not a blanket replacement for array access.
  *
 * Finally, you should NOT fix errors about array indexes in parts of the code
 * you don't understand by just replacing an array access with a call to idx().
 * This is sweeping the problem under the rug. Instead, you need to actually
 * understand the problem and determine the most appropriate solution. It is
 * possible that this really is idx(), but you can only make that determination
 * after understanding the context of the error.
 *
 * @param    array   Array to look for an index in.
 * @param    scalar  Index to check for. No longer accepts negative integers for
 *                   accessing elements from the end of the array.
 * @param    mixed   Default value to return if index is not found. By default,
 *                   this is null.
 * @return   mixed   Value at array index if it exists, or the default value
 *                   if not.
 */
function idx($arr, $idx, $default=null) {
  if (\is_array($arr)) {
    return \hphp_array_idx($arr, $idx, $default);
  }

  if ($idx !== null) {
    if (\is_object($arr)) {
      if ($arr instanceof \ConstIndexAccess) {
        if ($arr->containsKey($idx)) {
          return $arr[$idx];
        }
      } else if ($arr instanceof \ConstSet) {
        if ($arr->contains($idx)) {
          return $idx;
        }
      } else if ($arr instanceof \ArrayAccess) {
        if ($arr->offsetExists($idx)) {
          return $arr->offsetGet($idx);
        }
      }
    } else if (\is_string($arr)) {
      if (isset($arr[$idx])) {
        return $arr[$idx];
      }
    }
  }

  return $default;
}

}
