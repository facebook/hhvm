<?hh // partial
<<file:__EnableUnstableFeatures('readonly')>>

namespace {

const int ARRAY_FILTER_USE_BOTH = 1;
const int ARRAY_FILTER_USE_KEY = 2;

/**
 * Returns an array with all keys from input lowercased or uppercased.
 *   Numbered indices are left as is.
 *
 * @param mixed $input - The array to work on
 * @param int $case_ - Either CASE_UPPER or CASE_LOWER (default)
 *
 * @return mixed - Returns an array with its keys lower or uppercased, or
 *   FALSE if input is not an array.
 *
 */
<<__Native, __IsFoldable>>
function array_change_key_case(
  mixed $input,
  int $case_ = CASE_LOWER,
)[]: mixed;

/**
 * Chunks an array into size large chunks. The last chunk may contain less
 *   than size elements.
 *
 * @param mixed $input - The array to work on
 * @param int $size - The size of each chunk
 * @param bool $preserve_keys - When set to TRUE keys will be preserved.
 *   Default is FALSE which will reindex the chunk numerically
 *
 * @return mixed - Returns a multidimensional numerically indexed array,
 *   starting with zero, with each dimension containing size elements.
 *
 */
<<__Native, __IsFoldable>>
function array_chunk(
  mixed $input,
  int $size,
  bool $preserve_keys = false,
)[]: mixed;

/**
 * Return the values from a single column in the input array, identified by
 *   the value_key and optionally indexed by the index_key
 *
 * @param mixed $arr - Source array to pull column of values from
 * @param mixed $val_key - Key to pull values from in sub-arrays
 * @param mixed $idx_key - Key to pull indexs from in sub-arrays
 *
 * @return mixed - Returns the array column, or FALSE on failure
 *
 */
<<__Native, __IsFoldable>>
function array_column(
  mixed $arr,
  mixed $val_key,
  mixed $idx_key = null,
)[]: mixed;

/**
 * Creates an array by using the values from the keys array as keys and the
 *   values from the values array as the corresponding values.
 *
 * @param mixed $keys - Array of keys to be used. Illegal values for key will
 *   be converted to string.
 * @param mixed $values - Array of values to be used
 *
 * @return mixed - Returns the combined array, FALSE if the number of elements
 *   for each array isn't equal or if the arrays are empty.
 *
 */
<<__Native, __IsFoldable>>
function array_combine(
  mixed $keys,
  mixed $values,
)[]: mixed;

/**
 * array_count_values() returns an array using the values of the input array
 *   as keys and their frequency in input as values.
 *
 * @param AnyArray $input - The array of values to count
 *
 * @return mixed - Returns an associative array of values from input as keys
 *   and their count as value.
 *
 */
<<__Native, __IsFoldable>>
function array_count_values(AnyArray<arraykey, mixed> $input)[]: mixed;

/**
 * Fills an array with the value of the value parameter, using the values of
 *   the keys array as keys.
 *
 * @param mixed $keys - Array of values that will be used as keys. Illegal
 *   values for key will be converted to string.
 * @param mixed $value - Value to use for filling
 *
 * @return mixed - Returns the filled array
 *
 */
<<__Native, __IsFoldable>>
function array_fill_keys(
  mixed $keys,
  mixed $value,
)[]: darray<arraykey, mixed>;

/**
 * Fills an array with num entries of the value of the value parameter, keys
 *   starting at the start_index parameter.
 *
 * @param int $start_index - The first index of the returned array. Supports
 *   non-negative indexes only.
 * @param int $num - Number of elements to insert
 * @param mixed $value - Value to use for filling
 *
 * @return mixed - Returns the filled array
 *
 */
<<__Native, __IsFoldable>>
function array_fill(int $start_index, int $num, mixed $value)[]: mixed;

/**
 * array_flip() returns an array in flip order, i.e. keys from trans become
 *   values and values from trans become keys. Note that the values of trans
 *   need to be valid keys, i.e. they need to be either integer or string. A
 *   warning will be emitted if a value has the wrong type, and the key/value
 *   pair in question will not be flipped. If a value has several occurrences,
 *   the latest key will be used as its values, and all others will be lost.
 *
 * @param mixed $trans - An array of key/value pairs to be flipped.
 *
 * @return mixed - Returns the flipped array on success and NULL on failure.
 *
 */
<<__Native, __IsFoldable>>
function array_flip(
  mixed $trans,
)[]: mixed;

/**
 * array_key_exists() returns TRUE if the given key is set in the array. key
 *   can be any value possible for an array index.
 *
 * @param mixed $key - Value to check.
 * @param mixed $search - An array with keys to check.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native, __IsFoldable>>
function array_key_exists(
  readonly mixed $key,
  readonly mixed $search,
)[]: bool;

/**
 * key_exists() is the same as array_key_exists(). key_exists() returns TRUE
 *   if the given key is set in the array. key can be any value possible for
 *   an array index.
 *
 * @param mixed $key - Value to check.
 * @param mixed $search - An array with keys to check.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native, __IsFoldable>>
function key_exists(
  mixed $key,
  mixed $search,
)[]: bool;

/**
 * array_keys() returns the keys, numeric and string, from the input array.
 *   If the optional search_value is specified, then only the keys for that
 *   value are returned. Otherwise, all the keys from the input are returned.
 *
 * @param mixed $input - An array containing keys to return.
 * @param mixed $search_value - If specified, then only keys containing these
 *   values are returned.
 * @param bool $strict - Determines if strict comparison (===) should be used
 *   during the search.
 *
 * @return mixed - Returns an array of all the keys in input.
 *
 */
<<__Native, __IsFoldable>>
function array_keys(
  mixed $input,
)[]: mixed;

/**
 * array_merge_recursive() merges the elements of one or more arrays together
 *   so that the values of one are appended to the end of the previous one. It
 *   returns the resulting array. If the input arrays have the same string
 *   keys, then the values for these keys are merged together into an array, and
 *   this is done recursively, so that if one of the values is an array itself,
 *   the function will merge it with a corresponding entry in another array too.
 *   If, however, the arrays have the same numeric key, the later value will not
 *   overwrite the original value, but will be appended.
 *
 * @param mixed $array1 - Initial array to merge.
 * @param mixed $array2 - Second array to merge.
 *
 * @return mixed - An array of values resulted from merging the arguments
 *   together.
 *
 */
<<__Native, __IsFoldable>>
function array_merge_recursive(mixed $array1, mixed... $arrays)[]: mixed;

/**
 * Merges the elements of one or more arrays together so that the values of
 *   one are appended to the end of the previous one. It returns the resulting
 *   array. If the input arrays have the same string keys, then the later value
 *   for that key will overwrite the previous one. If, however, the arrays
 *   contain numeric keys, the later value will not overwrite the original
 *   value, but will be appended. If all of the arrays contain only numeric
 *   keys, the resulting array is given incrementing keys starting from zero.
 *
 * @param mixed $array1 - Initial array to merge.
 * @param mixed $array2 - Second array to merge.
 *
 * @return mixed - Returns the resulting array.
 *
 */
function array_merge(mixed $array1, mixed... $arrays)[]: mixed {
  // This is basically tvCastToArrayInPlace<..., IntishCast::Cast> with special
  // errors for inputs that are not Container<_>
  $arr1 = dict[];
  if ($array1 is keyset<_>) {
    foreach ($array1 as $k) {
      if ($k is string) {
        $intish = (int)$k;
        if ((string)$intish === $k) {
          $arr1[$intish] = $intish;
          continue;
        }
      }
      $arr1[$k] = $k;
    }
  } else if ($array1 is KeyedContainer<_, _>) {
    foreach ($array1 as $k => $v) {
      if ($k is string) {
        $intish = (int)$k;
        if ((string)$intish === $k) {
          $arr1[$intish] = $v;
          continue;
        }
      }
      $arr1[$k] = $v;
    }
  } else if (\HH\is_class_meth($array1)) {
    throw new InvalidOperationException("Cannot convert class method to array");
  } else {
    trigger_error(
      "Invalid operand type was used: array_merge expects array(s) or collection(s)",
      E_WARNING,
    );
    return null;
  }

  $ret = dict[];
  $nextKI = 0;
  foreach ($arr1 as $k => $v) {
    if ($k is int) {
      $ret[$nextKI] = $v;
      $nextKI++;
    } else {
      $ret[$k] = $v;
    }
  }
  foreach ($arrays as $arr) {
    if (!($arr is vec<_> || $arr is dict<_, _> || $arr is keyset<_>)) {
      trigger_error(
        "Invalid operand type was used: array_merge expects array(s)",
        E_WARNING,
      );
      return null;
    }
    foreach ($arr as $k => $v) {
      if ($k is int) {
        $ret[$nextKI] = $v;
        $nextKI++;
      } else {
        $ret[$k] = $v;
      }
    }
  }

  return $ret;
}

/**
 * array_replace_recursive() replaces the values of the first array with the
 *   same values from all the following arrays. If a key from the first array
 *   exists in the second array, its value will be replaced by the value from
 *   the second array. If the key exists in the second array, and not the first,
 *   it will be created in the first array. If a key only exists in the first
 *   array, it will be left as is. If several arrays are passed for replacement,
 *   they will be processed in order, the later array overwriting the previous
 *   values. array_replace_recursive() is recursive : it will recurse into
 *   arrays and apply the same process to the inner value. When the value in
 *   array is scalar, it will be replaced by the value in array1, may it be
 *   scalar or array. When the value in array and array1 are both arrays,
 *   array_replace_recursive() will replace their respective value recursively.
 *
 * @param mixed $array1 - The array in which elements are replaced.
 * @param mixed $array2 - The first array from which to replace values.
 *
 * @return mixed - Returns an array, or NULL if an error occurs.
 *
 */
<<__Native, __IsFoldable>>
function array_replace_recursive(
  mixed $array1,
  mixed $array2 = null,
  mixed... $argv
)[]: mixed;

/**
 * array_replace() replaces the values of the first array with the same values
 *   from all the following arrays. If a key from the first array exists in the
 *   second array, its value will be replaced by the value from the second
 *   array. If the key exists in the second array, and not the first, it will be
 *   created in the first array. If a key only exists in the first array, it
 *   will be left as is. If several arrays are passed for replacement, they will
 *   be processed in order, the later arrays overwriting the previous values.
 *   array_replace() is not recursive : it will replace values in the first
 *   array by whatever type is in the second array.
 *
 * @param mixed $array1 - The array in which elements are replaced.
 * @param mixed $array2 - The first array from which to replace values.
 *
 * @return mixed - Returns an array, or NULL if an error occurs.
 *
 */
<<__Native, __IsFoldable>>
function array_replace(mixed $array1, mixed $array2 = null, mixed... $argv)[]: mixed;

/**
 * array_pad() returns a copy of the input padded to size specified by
 *   pad_size with value pad_value. If pad_size is positive then the array is
 *   padded on the right, if it's negative then on the left. If the absolute
 *   value of pad_size is less than or equal to the length of the input then no
 *   padding takes place. It is possible to add most 1048576 elements at a time.
 *
 * @param mixed $input - Initial array of values to pad.
 * @param int $pad_size - New size of the array.
 * @param mixed $pad_value - Value to pad if input is less than pad_size.
 *
 * @return mixed - Returns a copy of the input padded to size specified by
 *   pad_size with value pad_value. If pad_size is positive then the array is
 *   padded on the right, if it's negative then on the left. If the absolute
 *   value of pad_size is less than or equal to the length of the input then no
 *   padding takes place.
 *
 */
<<__Native, __IsFoldable>>
function array_pad(
  mixed $input,
  int $pad_size,
  mixed $pad_value,
)[]: mixed;

/**
 * array_pop() pops and returns the last value of the array, shortening the
 *   array by one element. If array is empty (or is not an array), NULL will be
 *   returned. Will additionally produce a Warning when called on a non-array.
 *   This function will reset() the array pointer after use.
 *
 * @param mixed $array - The array to get the value from.
 *
 * @return mixed - Returns the last value of array. If array is empty (or is
 *   not an array), NULL will be returned.
 *
 */
<<__Native>>
function array_pop(
  inout mixed $array
)[write_props]: mixed;

/**
 * array_product() returns the product of values in an array.
 *
 * @param Container<T> $input - The input array or Collection.
 *
 * @return num - Returns the product as an integer or float.
 *
 */
<<__Native, __IsFoldable>>
function array_product(
  readonly mixed $input,
)[]: mixed;

/**
 * array_push() treats array as a stack, and pushes the passed variables onto
 *   the end. The length of array increases by the number of variables pushed.
 *   Has the same effect as "$array[] = $var;" repeated for each var. If you use
 *   array_push() to add one element to the array it's better to use $array[] =
 *   because in that way there is no overhead of calling a function.
 *   array_push() will raise a warning if the first argument is not a suitable
 *   container. This differs from the $var[] behaviour where a new array is
 *   created.
 *
 * @param mixed $array - The input array or collection.
 * @param mixed $var - The pushed value.
 *
 * @return mixed - Returns the new number of elements in the container.
 *
 */
<<__Native>>
function array_push(
  inout mixed $array,
  mixed $var,
  mixed... $args
)[]: mixed;

/**
 * Picks one ore more random entries out of an array, and returns the key (or
 *   keys) of the random entries.
 *
 * @param mixed $input - The input array.
 * @param int $num_req - Specifies how many entries you want to pick. Trying
 *   to pick more elements than there are in the array will result in an
 *   E_WARNING level error.
 *
 * @return mixed - If you are picking only one entry, array_rand() returns the
 *   key for a random entry. Otherwise, it returns an array of keys for the
 *   random entries. This is done so that you can pick random keys as well as
 *   values out of the array.
 *
 */
<<__Native>>
function array_rand(mixed $input, int $num_req = 1)[leak_safe]: mixed;

/**
 * array_reduce() applies iteratively the function function to the elements of
 *   the array input, so as to reduce the array to a single value.
 *
 * @param mixed $input - The input array.
 * @param mixed $callback - The callback function.
 * @param mixed $initial - If the optional initial is available, it will be
 *   used at the beginning of the process, or as a final result in case the
 *   array is empty.
 *
 * @return mixed - Returns the resulting value. If the array is empty and
 *   initial is not passed, array_reduce() returns NULL.
 *
 * Defined in array_reduce.hhas
 */


/**
 * Takes an input array and returns a new array with the order of the elements
 *   reversed.
 *
 * @param mixed $array - The input array.
 * @param bool $preserve_keys - If set to TRUE keys are preserved.
 *
 * @return ?array - Returns the reversed array.
 *
 */
<<__Native, __IsFoldable>>
function array_reverse(
  mixed $array,
  bool $preserve_keys = false,
)[]: ?darray<arraykey, mixed>;

/**
 * Searches haystack for needle.
 *
 * @param mixed $needle - The searched value. If needle is a string, the
 *   comparison is done in a case-sensitive manner.
 * @param mixed $haystack - The array.
 * @param bool $strict - If the third parameter strict is set to TRUE then the
 *   array_search() function will search for identical elements in the haystack.
 *   This means it will also check the types of the needle in the haystack, and
 *   objects must be the same instance.
 *
 * @return mixed - Returns the key for needle if it is found in the array,
 *   FALSE otherwise. If needle is found in haystack more than once, the first
 *   matching key is returned. To return the keys for all matching values, use
 *   array_keys() with the optional search_value parameter instead. WarningThis
 *   function may return Boolean FALSE, but may also return a non-Boolean value
 *   which evaluates to FALSE, such as 0 or "". Please read the section on
 *   Booleans for more information. Use the === operator for testing the return
 *   value of this function.
 *
 */
<<__Native, __IsFoldable>>
function array_search(
  mixed $needle,
  mixed $haystack,
  bool $strict = false,
)[]: mixed;

/**
 * array_shift() shifts the first value of the array off and returns it,
 *   shortening the array by one element and moving everything down. All
 *   numerical array keys will be modified to start counting from zero while
 *   literal keys won't be touched. This function will reset() the array pointer
 *   after use.
 *
 * @param mixed $array - The input array.
 *
 * @return mixed - Returns the shifted value, or NULL if array is empty or is
 *   not an array.
 *
 */
<<__Native>>
function array_shift(
  inout mixed $array
)[]: mixed;

/**
 * array_slice() returns the sequence of elements from the array array as
 *   specified by the offset and length parameters. This function will reset()
 *   the array pointer after use.
 *
 * @param mixed $array - The input array.
 * @param int $offset - If offset is non-negative, the sequence will start at
 *   that offset in the array. If offset is negative, the sequence will start
 *   that far from the end of the array.
 * @param mixed $length - If length is given and is positive, then the
 *   sequence will have that many elements in it. If length is given and is
 *   negative then the sequence will stop that many elements from the end of the
 *   array. If it is omitted, then the sequence will have everything from offset
 *   up until the end of the array.
 *
 * @param bool $preserve_keys - Note that array_slice() will reorder and reset
 *   the array indices by default. You can change this behaviour by setting
 *   preserve_keys to TRUE.
 *
 * @return mixed - Returns the slice.
 *
 */
<<__Native, __IsFoldable>>
function array_slice(
  mixed $array,
  int $offset,
  mixed $length = null,
  bool $preserve_keys = false,
)[]: mixed;

/**
 * Removes the elements designated by offset and length from the input array,
 *   and replaces them with the elements of the replacement array, if supplied.
 *   Note that numeric keys in input are not preserved. If replacement is not an
 *   array, it will be typecast to one (i.e. (array) $parameter). This may
 *   result in unexpected behavior when using an object or NULL replacement.
 *
 * @param mixed $input - The input array.
 * @param int $offset - If offset is positive then the start of removed
 *   portion is at that offset from the beginning of the input array. If offset
 *   is negative then it starts that far from the end of the input array.
 * @param mixed $length - If length is omitted, removes everything from offset
 *   to the end of the array. If length is specified and is positive, then that
 *   many elements will be removed. If length is specified and is negative then
 *   the end of the removed portion will be that many elements from the end of
 *   the array. Tip: to remove everything from offset to the end of the array
 *   when replacement is also specified, use count($input) for length.
 * @param mixed $replacement - If replacement array is specified, then the
 *   removed elements are replaced with elements from this array. If offset and
 *   length are such that nothing is removed, then the elements from the
 *   replacement array are inserted in the place specified by the offset. Note
 *   that keys in replacement array are not preserved. If replacement is just
 *   one element it is not necessary to put array() around it, unless the
 *   element is an array itself, an object or NULL.
 *
 * @return mixed - Returns the array consisting of the extracted elements.
 *
 */
<<__Native>>
function array_splice(
  inout mixed $input,
  int $offset,
  mixed $length = null,
  mixed $replacement = null,
)[]: mixed;

/**
 * array_sum() returns the sum of values in an array.
 *
 * @param Container<T> $input - The input array or Collection.
 *
 * @return num - Returns the sum of values as an integer or float.
 *
 */
<<__Native, __IsFoldable>>
function array_sum(
  readonly mixed $input,
)[]: mixed;

/**
 * Takes an input array and returns a new array without duplicate values.
 *   Note that keys are preserved. array_unique() sorts the values treated as
 *   string at first, then will keep the first key encountered for every value,
 *   and ignore all following keys. It does not mean that the key of the first
 *   related value from the unsorted array will be kept. Two elements are
 *   considered equal if and only if (string) $elem1 === (string) $elem2. In
 *   words: when the string representation is the same. The first element will
 *   be used.
 *
 * @param mixed $array - The input array.
 * @param int $sort_flags - The optional second parameter sort_flags may be
 *   used to modify the sorting behavior using these values: Sorting type
 *   flags: SORT_REGULAR - compare items normally (don't change types)
 *   SORT_NUMERIC - compare items numerically SORT_STRING - compare items as
 *   strings SORT_LOCALE_STRING - compare items as strings, based on the current
 *   locale.
 *
 * @return mixed - Returns the filtered array.
 *
 */
<<__Native, __IsFoldable>>
function array_unique(
  mixed $array,
  int $sort_flags = 2,
)[]: mixed;

/**
 * array_unshift() prepends passed elements to the front of the array. Note
 *   that the list of elements is prepended as a whole, so that the prepended
 *   elements stay in the same order. All numerical array keys will be modified
 *   to start counting from zero while literal keys won't be touched.
 *
 * @param mixed $array - The input array.
 * @param mixed $var - The prepended variable.
 *
 * @return mixed - Returns the new number of elements in the array.
 *
 */
<<__Native>>
function array_unshift(
  inout mixed $array,
  mixed $var,
  mixed... $argv
)[]: mixed;

/**
 * array_values() returns all the values from the input array and indexes
 *   numerically the array.
 *
 * @param mixed $input - The array.
 *
 * @return mixed - Returns an indexed array of values.
 *
 */
<<__Native, __IsFoldable>>
function array_values(
  mixed $input,
)[]: mixed;

/**
 * This function shuffles (randomizes the order of the elements in) an array.
 *
 * @param mixed $array - The array.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function shuffle(inout mixed $array)[leak_safe]: bool;

/**
 * Counts all elements in an array, or properties in an object. For objects,
 *   if you have SPL installed, you can hook into count() by implementing
 *   interface Countable. The interface has exactly one method, count(), which
 *   returns the return value for the count() function. Please see the Array
 *   section of the manual for a detailed explanation of how arrays are
 *   implemented and used in PHP.
 *
 * @param mixed $var - The array.
 * @param int $mode - If the optional mode parameter is set to COUNT_RECURSIVE
 *   (or 1), count() will recursively count the array. This is particularly
 *   useful for counting all the elements of a multidimensional array. count()
 *   does not detect infinite recursion.
 *
 * @return int - Returns the number of elements in var, which is typically an
 *   array, since anything else will have one element. If var is not an array
 *   or an object with implemented Countable interface, 1 will be returned.
 *   There is one exception, if var is NULL, 0 will be returned. Caution
 *   count() may return 0 for a variable that isn't set, but it may also return
 *   0 for a variable that has been initialized with an empty array. Use isset()
 *   to test if a variable is set.
 *
 * T35863429 for removing second arg
 */
<<__Native, __IsFoldable>>
function count(
  readonly mixed $var,
  int $mode = COUNT_NORMAL,
)[]: int;

/**
 * @param mixed $var
 * @return int
 */
<<__Native, __IsFoldable>>
function sizeof(
  readonly mixed $var,
)[]: int;

/**
 * Searches haystack for needle.
 *
 * @param mixed $needle - The searched value. If needle is a string, the
 *   comparison is done in a case-sensitive manner.
 * @param mixed $haystack - The array.
 *
 * @param bool $strict - If the third parameter strict is set to TRUE then the
 *   in_array() function will also check the types of the needle in the
 *   haystack.
 *
 * @return bool - Returns TRUE if needle is found in the array, FALSE
 *   otherwise.
 *
 */
<<__Native, __IsFoldable>>
function in_array(
  mixed $needle,
  mixed $haystack,
  bool $strict = false,
)[]: bool;

/**
 * Create an array containing a range of elements.
 *
 * @param mixed $low - Low value.
 * @param mixed $high - High value.
 * @param mixed $step - If a step value is given, it will be used as the
 *   increment between elements in the sequence. step should be given as a
 *   positive number. If not specified, step will default to 1.
 *
 * @return mixed - Returns an array of elements from low to high, inclusive.
 *   If low > high, the sequence will be from high to low.
 *
 */
<<__Native, __IsFoldable>>
function range(mixed $low, mixed $high, mixed $step = 1)[]: mixed;

/**
 * Compares container1 against container2 and returns the difference.
 *
 * @param mixed $container1 - The container to compare from
 * @param mixed $container2 - A container to compare against
 *
 * @return mixed - Returns an array containing all the entries from container1
 *   that are not present in any of the other containers.
 *
 */
<<__Native, __IsFoldable>>
function array_diff(mixed $container1, mixed $container2, mixed... $argv)[]: mixed;

/**
 * Computes the difference of arrays by using a callback function for data
 *   comparison. This is unlike array_diff() which uses an internal function for
 *   comparing the data.
 *
 * @param mixed $array1 - The first array.
 * @param mixed $array2 - The second array.
 * @param mixed $data_compare_func - The callback comparison function. The
 *   user supplied callback function is used for comparison. It must return an
 *   integer less than, equal to, or greater than zero if the first argument is
 *   considered to be respectively less than, equal to, or greater than the
 *   second.
 *
 * @return mixed - Returns an array containing all the values of array1 that
 *   are not present in any of the other arguments.
 *
 */
<<__Native>>
function array_udiff(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $data_compare_func,
  mixed... $argv
)[ctx $data_compare_func]: mixed;

/**
 * Compares array1 against array2 and returns the difference. Unlike
 *   array_diff() the array keys are used in the comparison.
 *
 * @param mixed $array1 - The array to compare from
 * @param mixed $array2 - An array to compare against
 *
 * @return mixed - Returns an array containing all the values from array1 that
 *   are not present in any of the other arrays.
 *
 */
<<__Native, __IsFoldable>>
function array_diff_assoc(mixed $array1, mixed $array2, mixed... $argv)[]: mixed;

/**
 * Compares array1 against array2 and returns the difference. Unlike
 *   array_diff() the array keys are used in the comparison. Unlike
 *   array_diff_assoc() an user supplied callback function is used for the
 *   indices comparison, not internal function.
 *
 * @param mixed $array1 - The array to compare from
 * @param mixed $array2 - An array to compare against
 * @param mixed $key_compare_func - More arrays to compare against
 *
 * @return mixed - Returns an array containing all the entries from array1
 *   that are not present in any of the other arrays.
 *
 */
<<__Native>>
function array_diff_uassoc(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $key_compare_func,
  mixed... $argv
)[ctx $key_compare_func]: mixed;

/**
 * Computes the difference of arrays with additional index check, compares
 *   data by a callback function. Please note that this function only checks one
 *   dimension of a n-dimensional array. Of course you can check deeper
 *   dimensions by using, for example, array_udiff_assoc($array1[0], $array2[0],
 *   "some_comparison_func");.
 *
 * @param mixed $array1 - The first array.
 * @param mixed $array2 - The second array.
 * @param mixed $data_compare_func - The callback comparison function. The
 *   user supplied callback function is used for comparison. It must return an
 *   integer less than, equal to, or greater than zero if the first argument is
 *   considered to be respectively less than, equal to, or greater than the
 *   second.
 *
 * @return mixed - array_udiff_assoc() returns an array containing all the
 *   values from array1 that are not present in any of the other arguments. Note
 *   that the keys are used in the comparison unlike array_diff() and
 *   array_udiff(). The comparison of arrays' data is performed by using an
 *   user-supplied callback. In this aspect the behaviour is opposite to the
 *   behaviour of array_diff_assoc() which uses internal function for
 *   comparison.
 *
 */
<<__Native>>
function array_udiff_assoc(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $data_compare_func,
  mixed... $argv
)[ctx $data_compare_func]: mixed;

/**
 * Computes the difference of arrays with additional index check, compares
 *   data and indexes by a callback function. Note that the keys are used in
 *   the comparison unlike array_diff() and array_udiff().
 *
 * @param mixed $array1 - The first array.
 * @param mixed $array2 - The second array.
 * @param mixed $data_compare_func - The callback comparison function. The
 *   user supplied callback function is used for comparison. It must return an
 *   integer less than, equal to, or greater than zero if the first argument is
 *   considered to be respectively less than, equal to, or greater than the
 *   second. The comparison of arrays' data is performed by using an
 *   user-supplied callback : data_compare_func. In this aspect the behaviour is
 *   opposite to the behaviour of array_diff_assoc() which uses internal
 *   function for comparison.
 * @param mixed $key_compare_func - The comparison of keys (indices) is done
 *   also by the callback function key_compare_func. This behaviour is unlike
 *   what array_udiff_assoc() does, since the latter compares the indices by
 *   using an internal function.
 *
 * @return mixed - Returns an array containing all the values from array1 that
 *   are not present in any of the other arguments.
 *
 */
<<__Native>>
function array_udiff_uassoc(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $data_compare_func,
  (function()[_]: void) $key_compare_func,
  mixed... $argv
)[ctx $data_compare_func, ctx $key_compare_func]: mixed;

/**
 * Compares the keys from container1 against the keys from container2 and
 *   returns the difference. This function is like array_diff() except the
 *   comparison is done on the keys instead of the values.
 *
 * @param mixed $container1 - The container to compare from
 * @param mixed $container2 - A container to compare against
 *
 * @return mixed - Returns an array containing all the entries from container1
 *   whose keys are not present in any of the other containers.
 *
 */
<<__Native, __IsFoldable>>
function array_diff_key(mixed $container1, mixed $container2, mixed... $argv)[]: mixed;

/**
 * Compares the keys from array1 against the keys from array2 and returns the
 *   difference. This function is like array_diff() except the comparison is
 *   done on the keys instead of the values. Unlike array_diff_key() an user
 *   supplied callback function is used for the indices comparison, not internal
 *   function.
 *
 * @param mixed $array1 - The array to compare from
 * @param mixed $array2 - An array to compare against
 * @param mixed $key_compare_func - More arrays to compare against
 *
 * @return mixed - Returns an array containing all the entries from array1
 *   that are not present in any of the other arrays.
 *
 */
<<__Native>>
function array_diff_ukey(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $key_compare_func,
  mixed... $argv
)[ctx $key_compare_func]: mixed;

/**
 * array_intersect() returns an array containing all the values of container1
 *   that are present in all the arguments. Note that keys are preserved.
 *
 * @param mixed $container1 - The container with master values to check.
 * @param mixed $container2 - A container to compare values against.
 *
 * @return mixed - Returns an array containing all of the values in container1
 *   whose values exist in all of the parameters.
 *
 */
<<__Native, __IsFoldable>>
function array_intersect(mixed $container1, mixed $container2, mixed... $argv)[]: mixed;

/**
 * Computes the intersection of arrays, compares data by a callback function.
 *
 * @param mixed $array1 - The first array.
 * @param mixed $array2 - The second array.
 * @param mixed $data_compare_func - The callback comparison function. The
 *   user supplied callback function is used for comparison. It must return an
 *   integer less than, equal to, or greater than zero if the first argument is
 *   considered to be respectively less than, equal to, or greater than the
 *   second.
 *
 * @return mixed - Returns an array containing all the values of array1 that
 *   are present in all the arguments.
 *
 */
<<__Native>>
function array_uintersect(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $data_compare_func,
  mixed... $argv
)[ctx $data_compare_func]: mixed;

/**
 * @param mixed $array1 - The array with master values to check.
 * @param mixed $array2 - An array to compare values against.
 *
 * @return mixed - Returns an associative array containing all the values in
 *   array1 that are present in all of the arguments.
 *
 */
<<__Native, __IsFoldable>>
function array_intersect_assoc(mixed $array1, mixed $array2, mixed... $argv)[]: mixed;

/**
 * array_intersect_uassoc() returns an array containing all the values of
 *   array1 that are present in all the arguments. Note that the keys are used
 *   in the comparison unlike in array_intersect(). The index comparison is
 *   done by a user supplied callback function. It must return an integer less
 *   than, equal to, or greater than zero if the first argument is considered to
 *   be respectively less than, equal to, or greater than the second.
 *
 * @param mixed $array1 - Initial array for comparison of the arrays.
 * @param mixed $array2 - First array to compare keys against.
 * @param mixed $key_compare_func - Variable list of array arguments to
 *   compare values against.
 *
 * @return mixed - Returns the values of array1 whose values exist in all of
 *   the arguments.
 *
 */
<<__Native>>
function array_intersect_uassoc(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $key_compare_func,
  mixed... $argv
)[ctx $key_compare_func]: mixed;

/**
 * Computes the intersection of arrays with additional index check, compares
 *   data by a callback function. Note that the keys are used in the comparison
 *   unlike in array_uintersect(). The data is compared by using a callback
 *   function.
 *
 * @param mixed $array1 - The first array.
 * @param mixed $array2 - The second array.
 * @param mixed $data_compare_func - For comparison is used the user supplied
 *   callback function. It must return an integer less than, equal to, or
 *   greater than zero if the first argument is considered to be respectively
 *   less than, equal to, or greater than the second.
 *
 * @return mixed - Returns an array containing all the values of array1 that
 *   are present in all the arguments.
 *
 */
<<__Native>>
function array_uintersect_assoc(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $data_compare_func,
  mixed... $argv
)[ctx $data_compare_func]: mixed;

/**
 * Computes the intersection of arrays with additional index check, compares
 *   data and indexes by a callback functions Note that the keys are used in the
 *   comparison unlike in array_uintersect(). Both the data and the indexes are
 *   compared by using separate callback functions.
 *
 * @param mixed $array1 - The first array.
 * @param mixed $array2 - The second array.
 * @param mixed $data_compare_func - For comparison is used the user supplied
 *   callback function. It must return an integer less than, equal to, or
 *   greater than zero if the first argument is considered to be respectively
 *   less than, equal to, or greater than the second.
 * @param mixed $key_compare_func - Key comparison callback function.
 *
 * @return mixed - Returns an array containing all the values of array1 that
 *   are present in all the arguments.
 *
 */
<<__Native>>
function array_uintersect_uassoc(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $data_compare_func,
  (function()[_]: void) $key_compare_func,
  mixed...$argv
)[ctx $data_compare_func, ctx $key_compare_func]: mixed;

/**
 * array_intersect_key() returns an array containing all the entries of
 *   container1 which have keys that are present in all the arguments.
 *
 * @param mixed $container1 - The container with master keys to check.
 * @param mixed $container2 - A container to compare keys against.
 *
 * @return mixed - Returns an array containing all the entries of container1
 *   which have keys that are present in all arguments.
 *
 */
<<__Native, __IsFoldable>>
function array_intersect_key(
  mixed $container1,
  mixed $container2,
  mixed... $argv
)[]: mixed;

/**
 * array_intersect_ukey() returns an array containing all the values of array1
 *   which have matching keys that are present in all the arguments. This
 *   comparison is done by a user supplied callback function. It must return an
 *   integer less than, equal to, or greater than zero if the first key is
 *   considered to be respectively less than, equal to, or greater than the
 *   second.
 *
 * @param mixed $array1 - Initial array for comparison of the arrays.
 * @param mixed $array2 - First array to compare keys against.
 * @param mixed $key_compare_func - Variable list of array arguments to
 *   compare keys against.
 *
 * @return mixed - Returns the values of array1 whose keys exist in all the
 *   arguments.
 *
 */
<<__Native>>
function array_intersect_ukey(
  mixed $array1,
  mixed $array2,
  (function()[_]: void) $key_compare_func,
  mixed... $argv
)[ctx $key_compare_func]: mixed;

/**
 * This function sorts an array. Elements will be arranged from lowest to
 *   highest when this function has completed.
 *
 * @param mixed $array - The input array.
 * @param int $sort_flags - The optional second parameter sort_flags may be
 *   used to modify the sorting behavior using these values: Sorting type
 *   flags: SORT_REGULAR - compare items normally (don't change types)
 *   SORT_NUMERIC - compare items numerically SORT_STRING - compare items as
 *   strings SORT_LOCALE_STRING - compare items as strings, based on the current
 *   locale. Added in PHP 4.4.0 and 5.0.2, it uses the system locale, which can
 *   be changed using setlocale().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function sort(
  inout mixed $array,
  int $sort_flags = 0,
)[]: bool;

/**
 * This function sorts an array in reverse order (highest to lowest).
 *
 * @param mixed $array - The input array.
 * @param int $sort_flags - You may modify the behavior of the sort using the
 *   optional parameter sort_flags, for details see sort().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function rsort(
  inout mixed $array,
  int $sort_flags = 0,
)[]: bool;

/**
 * This function sorts an array such that array indices maintain their
 *   correlation with the array elements they are associated with. This is used
 *   mainly when sorting associative arrays where the actual element order is
 *   significant.
 *
 * @param mixed $array - The input array.
 * @param int $sort_flags - You may modify the behavior of the sort using the
 *   optional parameter sort_flags, for details see sort().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function asort(
  inout mixed $array,
  int $sort_flags = 0,
)[]: bool;

/**
 * This function sorts an array such that array indices maintain their
 *   correlation with the array elements they are associated with. This is used
 *   mainly when sorting associative arrays where the actual element order is
 *   significant.
 *
 * @param mixed $array - The input array.
 * @param int $sort_flags - You may modify the behavior of the sort using the
 *   optional parameter sort_flags, for details see sort().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function arsort(
  inout mixed $array,
  int $sort_flags = 0,
)[]: bool;

/**
 * Sorts an array by key, maintaining key to data correlations. This is useful
 *   mainly for associative arrays.
 *
 * @param mixed $array - The input array.
 * @param int $sort_flags - You may modify the behavior of the sort using the
 *   optional parameter sort_flags, for details see sort().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ksort(
  inout mixed $array,
  int $sort_flags = 0,
)[]: bool;

/**
 * Sorts an array by key in reverse order, maintaining key to data
 *   correlations. This is useful mainly for associative arrays.
 *
 * @param mixed $array - The input array.
 * @param int $sort_flags - You may modify the behavior of the sort using the
 *   optional parameter sort_flags, for details see sort().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function krsort(
  inout mixed $array,
  int $sort_flags = 0,
)[]: bool;

/**
 * This function will sort an array by its values using a user-supplied
 *   comparison function. If the array you wish to sort needs to be sorted by
 *   some non-trivial criteria, you should use this function. If two members
 *   compare as equal, their order in the sorted array is undefined. This
 *   function assigns new keys to the elements in array. It will remove any
 *   existing keys that may have been assigned, rather than just reordering the
 *   keys.
 *
 * @param mixed $array - The input array.
 * @param mixed $cmp_function - The comparison function must return an integer
 *   less than, equal to, or greater than zero if the first argument is
 *   considered to be respectively less than, equal to, or greater than the
 *   second.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function usort(
  inout mixed $array,
  (function(arraykey, arraykey)[_]: int) $cmp_function,
)[ctx $cmp_function]: bool;

/**
 * This function sorts an array such that array indices maintain their
 *   correlation with the array elements they are associated with, using a
 *   user-defined comparison function. This is used mainly when sorting
 *   associative arrays where the actual element order is significant.
 *
 * @param mixed $array - The input array.
 * @param mixed $cmp_function - See usort() and uksort() for examples of
 *   user-defined comparison functions.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function uasort(
  inout mixed $array,
  (function(arraykey, arraykey)[_]: int) $cmp_function,
)[ctx $cmp_function]: bool;

/**
 * uksort() will sort the keys of an array using a user-supplied comparison
 *   function. If the array you wish to sort needs to be sorted by some
 *   non-trivial criteria, you should use this function.
 *
 * @param mixed $array - The input array.
 * @param mixed $cmp_function - The callback comparison function. Function
 *   cmp_function should accept two parameters which will be filled by pairs of
 *   array keys. The comparison function must return an integer less than, equal
 *   to, or greater than zero if the first argument is considered to be
 *   respectively less than, equal to, or greater than the second.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function uksort(
  inout mixed $array,
  (function(arraykey, arraykey)[_]: int) $cmp_function,
)[ctx $cmp_function]: bool;

/**
 * This function implements a sort algorithm that orders alphanumeric strings
 *   in the way a human being would while maintaining key/value associations.
 *   This is described as a "natural ordering". An example of the difference
 *   between this algorithm and the regular computer string sorting algorithms
 *   (used in sort()) can be seen in the example below.
 *
 * @param mixed $array - The input array.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function natsort(inout mixed $array)[]: bool;

/**
 * natcasesort() is a case insensitive version of natsort(). This function
 *   implements a sort algorithm that orders alphanumeric strings in the way a
 *   human being would while maintaining key/value associations. This is
 *   described as a "natural ordering".
 *
 * @param mixed $array - The input array.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function natcasesort(inout mixed $array)[]: bool;

<<__Native>>
function i18n_loc_get_default(): string;

<<__Native>>
function i18n_loc_set_default(string $locale): bool;

<<__Native>>
function i18n_loc_set_attribute(int $attr, int $val): bool;

<<__Native>>
function i18n_loc_set_strength(int $strength): bool;

<<__Native>>
function i18n_loc_get_error_code(): mixed;

/**
 * hphp_array_idx() returns the value at the given key in the given array or
 *   the given default value if it is not found. An error will be raised if the
 *   search parameter is not an array.
 *
 * @param mixed $search - An array with keys to check.
 * @param mixed $key - Value to check.
 * @param mixed $def - The value to return if key is not found in search.
 *
 * @return mixed - Returns the value at 'key' in 'search' or 'def' if it is
 *   not found.
 *
 */
<<__Native, __IsFoldable>>
function hphp_array_idx(
  mixed $search,
  mixed $key,
  mixed $def,
)[]: mixed;

/**
 * array_multisort() can be used to sort several arrays at once, or a
 *   multi-dimensional array by one or more dimensions.  Associative (string)
 *   keys will be maintained, but numeric keys will be re-indexed.
 *
 * @param mixed $arg1 - An array being sorted.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function array_multisort1(
  inout mixed $arg1,
): bool;

<<__Native>>
function array_multisort2(
  inout mixed $arg1,
  inout mixed $arg2,
): bool;

<<__Native>>
function array_multisort3(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
): bool;

<<__Native>>
function array_multisort4(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
): bool;

<<__Native>>
function array_multisort5(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
): bool;

<<__Native>>
function array_multisort6(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
  inout mixed $arg6,
): bool;

<<__Native>>
function array_multisort7(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
  inout mixed $arg6,
  inout mixed $arg7,
): bool;

<<__Native>>
function array_multisort8(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
  inout mixed $arg6,
  inout mixed $arg7,
  inout mixed $arg8,
): bool;

<<__Native>>
function array_multisort9(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
  inout mixed $arg6,
  inout mixed $arg7,
  inout mixed $arg8,
  inout mixed $arg9,
): bool;

} // root namespace

namespace __SystemLib {
  /* array_map() returns an array containing all the elements of arr1 after
   * applying the callback function to each one. The number of parameters that
   * the callback function accepts should match the number of arrays passed to
   * the array_map()
   * @param mixed $callback - Callback function to run for each element in each
   * array.
   * @param mixed $arr1 - An array to run through the callback function.
   * @return mixed - Returns an array containing all the elements of arr1 after
   * applying the callback function to each one.
   *
   * SystemLib defines the HHAS fast-path for array_map() as taking two args.
   * First is valid callback, second is valid array.
   *
   * If array_map() is called by other means, it dispatches to this version
   * which allows variadic array counts and deals with bad types.
   */
  <<__Native>>
  function array_map(mixed $callback, mixed $arr1, mixed ...$argv): mixed;

  <<__Native, __IsFoldable>>
  function merge_xhp_attr_declarations(
    darray<arraykey, mixed> $arr1,
    darray<arraykey, mixed> $arr2,
    darray<arraykey, mixed>... $rest
  )[]: darray<arraykey, mixed>;
}

namespace HH {
  <<__Native, __IsFoldable>>
  function dict<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $arr,
  )[]: dict<Tk, Tv>;

  <<__Native, __IsFoldable>>
  function vec<T>(
    Traversable<T> $arr,
  )[]: vec<T>;

  <<__Native, __IsFoldable>>
  function keyset<T as arraykey>(
    Traversable<T> $arr,
  )[]: keyset<T>;

  <<__Native, __IsFoldable>>
  function varray<T>(
    Traversable<T> $arr,
  )[]: varray<T>;

  <<__Native, __IsFoldable>>
  function darray<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $arr,
  )[]: darray<Tk, Tv>;

  /**
   * array_key_cast() can be used to convert a given value to the equivalent
   * that would be used if that value was used as a key in an array.
   *
   * An integer is returned unchanged. A boolean, float, or resource is cast to
   * an integer (using standard semantics). A null is converted to an empty
   * string. A string is converted to an integer if it represents an integer
   * value, returned unchanged otherwise.
   *
   * For object, array, vec, dict, or keyset values, an InvalidArgumentException
   * is thrown (as these cannot be used as array keys).
   *
   * @param mixed $key - The value to be converted.
   *
   * @return arraykey - Returns the converted value.
   */
  <<__Native, __IsFoldable>>
  function array_key_cast(mixed $key)[]: arraykey;
}
