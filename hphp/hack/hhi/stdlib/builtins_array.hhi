<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// flags for array_change_key_case()
const int CASE_LOWER;
const int CASE_UPPER;

// flags for array_multisort
const int SORT_ASC;
const int SORT_DESC;

// flags for sort() family
const int SORT_REGULAR;
const int SORT_NUMERIC;
const int SORT_STRING;
const int SORT_LOCALE_STRING;
const int SORT_NATURAL;
const int SORT_FLAG_CASE;

// flags for count()
const int COUNT_NORMAL;
const int COUNT_RECURSIVE;

const int UCOL_DEFAULT;
const int UCOL_PRIMARY;
const int UCOL_SECONDARY;
const int UCOL_TERTIARY;
const int UCOL_DEFAULT_STRENGTH;
const int UCOL_QUATERNARY;
const int UCOL_IDENTICAL;
const int UCOL_OFF;
const int UCOL_ON;
const int UCOL_SHIFTED;
const int UCOL_NON_IGNORABLE;
const int UCOL_LOWER_FIRST;
const int UCOL_UPPER_FIRST;
const int UCOL_FRENCH_COLLATION;
const int UCOL_ALTERNATE_HANDLING;
const int UCOL_CASE_FIRST;
const int UCOL_CASE_LEVEL;
const int UCOL_NORMALIZATION_MODE;
const int UCOL_STRENGTH;
const int UCOL_HIRAGANA_QUATERNARY_MODE;
const int UCOL_NUMERIC_COLLATION;

const int TAG_PROVENANCE_HERE_MUTATE_COLLECTIONS;

<<__PHPStdLib>>
function array_change_key_case<Tv>(
  KeyedContainer<arraykey, Tv> $input,
  int $upper = CASE_LOWER,
)[]: darray<arraykey, Tv>;
/** inner container will be a varray<mixed> or darray<arraykey, mixed> depending on $preserve_keys */
<<__PHPStdLib>>
function array_chunk<Tv>(
  Container<Tv> $input,
  int $size,
  bool $preserve_keys = false,
)[]: varray<Container<int>>;
<<__PHPStdLib>>
function array_combine<Tv1 as arraykey, Tv2>(
  Container<Tv1> $keys,
  Container<Tv2> $values,
)[]: darray<Tv1, Tv2>;
<<__PHPStdLib>>
function array_count_values<Tv as arraykey>(
  Container<Tv> $input,
)[]: darray<Tv, int>;
<<__PHPStdLib>>
function array_column<Tv>(
  Container<KeyedContainer<arraykey, Tv>> $array,
  ?arraykey $column_key,
)[]: varray_or_darray<Tv>;
<<__PHPStdLib>>
function array_fill_keys<Tk as arraykey, Tv>(
  Container<Tk> $keys,
  Tv $value,
)[]: darray<Tk, Tv>;
/**
 * `array_filter` returns a `KeyedContainer<_, _>` with the items that match
 * the predicate.
 *
 * This function is deprecated; please use `Dict\filter` or `Vec\filter`
 * instead, or the respective `filter_nulls` variant.
 *
 * `array_filter` used to have special typing to indicate:
 *   - Whether or not it preserved the input type
 *   - How it removed nulls upon not providing a callback
 *
 * These typing behaviors are now split into the aforementioned four Hack
 * Standard Library functions.
 */
<<__PHPStdLib>>
function array_filter<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $input,
  ?(function(Tv): bool) $callback = null,
): KeyedContainer<Tk, Tv>;
<<__PHPStdLib>>
function array_flip(
  HH\FIXME\MISSING_PARAM_TYPE $trans,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function key_exists(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $search,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_keys<Tk as arraykey>(
  KeyedContainer<Tk, mixed> $input,
)[]: varray<Tk>;
/**
 * `array_map` previously had it's signature rewritten based on the arity of
 * the call, to match runtime behaviors including:
 * - Preserving the input container type
 * - Allowing for `N` args and accepting a function of the same arity
 *
 * This runtime behavior still exists but this function is deprecated in favor
 * of HSL functions like `Vec\map` or `Dict\map`.
 */
<<__PHPStdLib>>
function array_map<Tk as arraykey, Tin, Tout>(
  (function(Tin): Tout) $callback,
  KeyedContainer<Tk, Tin> $arr,
): KeyedContainer<Tk, Tout>;

<<__PHPStdLib>>
function array_merge_recursive(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_merge(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_replace_recursive(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_replace(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort1(inout mixed $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort2(
  inout mixed $arg1,
  inout mixed $arg2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort3(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort4(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort5(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort6(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
  inout mixed $arg6,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort7(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
  inout mixed $arg6,
  inout mixed $arg7,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_multisort8(
  inout mixed $arg1,
  inout mixed $arg2,
  inout mixed $arg3,
  inout mixed $arg4,
  inout mixed $arg5,
  inout mixed $arg6,
  inout mixed $arg7,
  inout mixed $arg8,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
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
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_pad(
  HH\FIXME\MISSING_PARAM_TYPE $input,
  int $pad_size,
  HH\FIXME\MISSING_PARAM_TYPE $pad_value,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_pop(
  inout HH\FIXME\MISSING_PARAM_TYPE $array,
)[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_push(
  inout HH\FIXME\MISSING_PARAM_TYPE $array,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_rand(
  HH\FIXME\MISSING_PARAM_TYPE $input,
  int $num_req = 1,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_reduce(
  HH\FIXME\MISSING_PARAM_TYPE $input,
  HH\FIXME\MISSING_PARAM_TYPE $callback,
  HH\FIXME\MISSING_PARAM_TYPE $initial = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_reverse(
  HH\FIXME\MISSING_PARAM_TYPE $array,
  bool $preserve_keys = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_search(
  HH\FIXME\MISSING_PARAM_TYPE $needle,
  HH\FIXME\MISSING_PARAM_TYPE $haystack,
  bool $strict = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_shift<T>(inout T $array)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_slice(
  HH\FIXME\MISSING_PARAM_TYPE $array,
  int $offset,
  HH\FIXME\MISSING_PARAM_TYPE $length = null,
  bool $preserve_keys = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_splice(
  inout mixed $input,
  int $offset,
  mixed $length = null,
  mixed $replacement = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_unique(
  HH\FIXME\MISSING_PARAM_TYPE $array,
  int $sort_flags = 2,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_unshift<T>(
  inout T $array,
  HH\FIXME\MISSING_PARAM_TYPE $var,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_values<Tv>(Container<Tv> $input)[]: varray<Tv>;
<<__PHPStdLib>>
function shuffle<T>(inout T $array)[leak_safe]: void;
<<
  __Deprecated(
    'Use count(), it does the same thing as sizeof() in PHP and '.
    'doesn\'t suggest that it\'s counting bytes.',
  ),
  __PHPStdLib,
>>
function sizeof(
  HH\FIXME\MISSING_PARAM_TYPE $var,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphp_get_iterator(
  HH\FIXME\MISSING_PARAM_TYPE $iterable,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function in_array(
  HH\FIXME\MISSING_PARAM_TYPE $needle,
  HH\FIXME\MISSING_PARAM_TYPE $haystack,
  bool $strict = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function range(
  HH\FIXME\MISSING_PARAM_TYPE $low,
  HH\FIXME\MISSING_PARAM_TYPE $high,
  HH\FIXME\MISSING_PARAM_TYPE $step = 1,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_diff(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_udiff(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $data_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_diff_assoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_diff_uassoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $key_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_udiff_assoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $data_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_udiff_uassoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $data_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE $key_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_diff_key(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_diff_ukey(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $key_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_intersect(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_uintersect(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $data_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_intersect_assoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_intersect_uassoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $key_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_uintersect_assoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $data_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_uintersect_uassoc(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $data_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE $key_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_intersect_key(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function array_intersect_ukey(
  HH\FIXME\MISSING_PARAM_TYPE $array1,
  HH\FIXME\MISSING_PARAM_TYPE $array2,
  HH\FIXME\MISSING_PARAM_TYPE $key_compare_func,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function natsort(
  inout HH\FIXME\MISSING_PARAM_TYPE $array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function natcasesort(
  inout HH\FIXME\MISSING_PARAM_TYPE $array,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function i18n_loc_get_default(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function i18n_loc_set_default(string $locale): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function i18n_loc_set_attribute(
  int $attr,
  int $val,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function i18n_loc_set_strength(int $strength): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function i18n_loc_get_error_code(): HH\FIXME\MISSING_RETURN_TYPE;
