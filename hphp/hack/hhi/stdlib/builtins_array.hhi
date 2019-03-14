<?hh    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// flags for array_change_key_case()
const int CASE_LOWER = 0;
const int CASE_UPPER = 0;

// flags for array_multisort
const int SORT_ASC = 0;
const int SORT_DESC = 0;

// flags for sort() family
const int SORT_REGULAR = 0;
const int SORT_NUMERIC = 0;
const int SORT_STRING = 0;
const int SORT_LOCALE_STRING = 0;
const int SORT_NATURAL = 0;
const int SORT_FLAG_CASE = 0;

// flags for count()
const int COUNT_NORMAL = 0;
const int COUNT_RECURSIVE = 0;

const int UCOL_DEFAULT = 0;
const int UCOL_PRIMARY = 0;
const int UCOL_SECONDARY = 0;
const int UCOL_TERTIARY = 0;
const int UCOL_DEFAULT_STRENGTH = 0;
const int UCOL_QUATERNARY = 0;
const int UCOL_IDENTICAL = 0;
const int UCOL_OFF = 0;
const int UCOL_ON = 0;
const int UCOL_SHIFTED = 0;
const int UCOL_NON_IGNORABLE = 0;
const int UCOL_LOWER_FIRST = 0;
const int UCOL_UPPER_FIRST = 0;
const int UCOL_FRENCH_COLLATION = 0;
const int UCOL_ALTERNATE_HANDLING = 0;
const int UCOL_CASE_FIRST = 0;
const int UCOL_CASE_LEVEL = 0;
const int UCOL_NORMALIZATION_MODE = 0;
const int UCOL_STRENGTH = 0;
const int UCOL_HIRAGANA_QUATERNARY_MODE = 0;
const int UCOL_NUMERIC_COLLATION = 0;
<<__PHPStdLib, __Rx>>
function array_change_key_case($input, int $upper = CASE_LOWER);
<<__PHPStdLib, __Rx>>
function array_chunk($input, int $size, bool $preserve_keys = false);
<<__PHPStdLib, __Rx>>
function array_combine($keys, $values);
<<__PHPStdLib, __Rx>>
function array_count_values($input);
<<__PHPStdLib, __Rx>>
function array_column<Tk as arraykey, Tv>(
  array<array<Tk, Tv>> $array,
  ?Tk $column_key,
  ?Tk $index_key = null,
): array;
<<__PHPStdLib, __Rx>>
function array_fill_keys($keys, $value);
/*
 * Calls to array_filter are rewritten depending on the type
 * of argument to have one of the following signatures:
 *
 * function(array, ?(function(Tv):bool)): array
 * function(KeyedContainer<Tk, Tv>, ?(function(Tv):bool)): array<Tk, Tv>
 * function(Container<Tv>, ?(function(Tv):bool)): array<arraykey, Tv>
 *
 * Single argument calls additionally remove nullability of Tv, i.e.:
 *
 * function(Container<?Tv>): array<arraykey, Tv>
 *
 */
<<__PHPStdLib>>
function array_filter<Tv>(Container<Tv> $input, ?(function(Tv):bool) $callback = null);
<<__PHPStdLib, __Rx>>
function array_flip($trans);
<<__PHPStdLib, __Rx>>
function key_exists($key, $search);
<<__PHPStdLib, __Rx>>
function array_keys<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $input,
): varray<Tk>;
/**
 * array_map signature is rewritten based on the arity of the call:
 *
 * array_map(F, A1, A2, ..., An); becomes
 *
 * array_map<T1, ... Tn, Tr>(
 *   (function(T1, ..., Tn): Tr),
 *   Container<T1>,
 *   ...,
 *   Container<Tn>
 *): R;
 *
 * where for n > 1, R = array<Tr>
 * for n = 1, R depends on actual type of container passed at the call site:
 *
 * array                 -> R = array
 * array<X>              -> R = array<Tr>
 * array<X, Y>           -> R = array<X, Tr>
 * Vector<X>             -> R = array<Tr>
 * KeyedContainer<X, Y>  -> R = array<X, Tr>
 * Container<X>          -> R = array<arraykey, Tr>
 * X (unknown type)      -> R = Y (other unknown type)
 */
<<__PHPStdLib>>
function array_map($callback, $arr1, ...);
<<__PHPStdLib, __Rx>>
function array_merge_recursive($array1, ...);
<<__PHPStdLib, __Rx>>
function array_merge($array1, ...);
<<__Rx>>
function array_replace_recursive($array1, ...);
<<__Rx>>
function array_replace($array1, ...);
/* HH_IGNORE_ERROR[3068] this is a builtin */
function array_multisort(&$arr1, &...$rest);
<<__PHPStdLib, __Rx>>
function array_pad($input, int $pad_size, $pad_value);
<<__PHPStdLib>>
function array_pop(&$array);
<<__PHPStdLib>>
function array_push(&$array, $var, ...);
<<__PHPStdLib>>
function array_rand($input, int $num_req = 1);
function array_reduce($input, $callback, $initial = null);
<<__PHPStdLib, __Rx>>
function array_reverse($array, bool $preserve_keys = false);
<<__PHPStdLib, __Rx>>
function array_search($needle, $haystack, bool $strict = false);
<<__PHPStdLib>>
function array_shift(&$array);
<<__PHPStdLib, __Rx>>
function array_slice($array, int $offset, $length = null, bool $preserve_keys = false);
<<__PHPStdLib>>
function array_splice(&$input, int $offset, $length = null, $replacement = null);
<<__PHPStdLib, __Rx>>
function array_unique($array, int $sort_flags = 2);
<<__PHPStdLib>>
function array_unshift(&$array, $var, ...);
<<__PHPStdLib, __Rx>>
function array_values<Tv>(Container<Tv> $input): array<Tv>;
<<__PHPStdLib, __Deprecated('This function is scheduled for removal')>>
function array_walk_recursive(&$input, $funcname, $userdata = null);
<<__PHPStdLib, __Deprecated('This function is scheduled for removal')>>
function array_walk(&$input, $funcname, $userdata = null);
<<__PHPStdLib>>
function shuffle(&$array);
<<__Deprecated('Use count(), it does the same thing as sizeof() in PHP and '.
  'doesn\'t suggest that it\'s counting bytes.'), __PHPStdLib, __Rx>>
function sizeof($var, int $recursive = COUNT_NORMAL);
<<__PHPStdLib>>
function each(&$array);
<<__PHPStdLib>>
function current(&$array);
<<__PHPStdLib>>
function hphp_current_ref(&$array);
<<__PHPStdLib>>
function next(&$array);
<<__PHPStdLib>>
function pos(&$array);
<<__PHPStdLib>>
function prev(&$array);
<<__PHPStdLib>>
function reset(&$array);
<<__PHPStdLib>>
function end(&$array);
<<__PHPStdLib>>
function key(&$array);
<<__PHPStdLib>>
function hphp_get_iterator($iterable);
<<__PHPStdLib>>
function hphp_get_mutable_iterator(&$iterable);
<<__PHPStdLib, __Rx>>
function in_array($needle, $haystack, bool $strict = false);
<<__PHPStdLib, __Rx>>
function range($low, $high, $step = 1);
<<__PHPStdLib, __Rx>>
function array_diff($array1, $array2, ...);
<<__PHPStdLib>>
function array_udiff($array1, $array2, $data_compare_func, ...);
<<__PHPStdLib, __Rx>>
function array_diff_assoc($array1, $array2, ...);
<<__PHPStdLib>>
function array_diff_uassoc($array1, $array2, $key_compare_func, ...);
<<__PHPStdLib>>
function array_udiff_assoc($array1, $array2, $data_compare_func, ...);
<<__PHPStdLib>>
function array_udiff_uassoc($array1, $array2, $data_compare_func, $key_compare_func, ...);
<<__PHPStdLib, __Rx>>
function array_diff_key($array1, $array2, ...);
<<__PHPStdLib>>
function array_diff_ukey($array1, $array2, $key_compare_func, ...);
<<__PHPStdLib, __Rx>>
function array_intersect($array1, $array2, ...);
<<__PHPStdLib>>
function array_uintersect($array1, $array2, $data_compare_func, ...);
<<__PHPStdLib, __Rx>>
function array_intersect_assoc($array1, $array2, ...);
<<__PHPStdLib>>
function array_intersect_uassoc($array1, $array2, $key_compare_func, ...);
<<__PHPStdLib>>
function array_uintersect_assoc($array1, $array2, $data_compare_func, ...);
<<__PHPStdLib>>
function array_uintersect_uassoc($array1, $array2, $data_compare_func, $key_compare_func, ...);
<<__PHPStdLib, __Rx>>
function array_intersect_key($array1, $array2, ...);
<<__PHPStdLib>>
function array_intersect_ukey($array1, $array2, $key_compare_func, ...);
<<__PHPStdLib>>
function natsort(&$array);
<<__PHPStdLib>>
function natcasesort(&$array);
<<__PHPStdLib>>
function i18n_loc_get_default();
<<__PHPStdLib>>
function i18n_loc_set_default(string $locale);
<<__PHPStdLib>>
function i18n_loc_set_attribute(int $attr, int $val);
<<__PHPStdLib>>
function i18n_loc_set_strength(int $strength);
<<__PHPStdLib>>
function i18n_loc_get_error_code();
