<?hh // decl    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

// flags for extract()
const int EXTR_OVERWRITE = 0;
const int EXTR_SKIP = 0;
const int EXTR_PREFIX_SAME = 0;
const int EXTR_PREFIX_ALL = 0;
const int EXTR_PREFIX_INVALID = 0;
const int EXTR_PREFIX_IF_EXISTS = 0;
const int EXTR_IF_EXISTS = 0;
const int EXTR_REFS = 0;

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
function array_change_key_case($input, $upper = false);
function array_chunk($input, $size, $preserve_keys = false);
function array_combine($keys, $values);
function array_count_values($input);
function array_column<Tk as arraykey, Tv>(
  array<array<Tk, Tv>> $array,
  Tk $column_key,
  ?Tk $index_key = null,
): array;
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
function array_filter<Tv>(Container<Tv> $input, ?(function(Tv):bool) $callback = null);
function array_flip($trans);
function key_exists($key, $search);
function array_keys<Tk, Tv>(
  KeyedContainer<Tk, Tv> $input,
  ?Tv $search_value = null,
  bool $strict = false
): array<Tk>;
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
function array_map($callback, $arr1, ...);
function array_merge_recursive($array1, ...);
function array_merge($array1, ...);
function array_replace_recursive($array1, ...);
function array_replace($array1, ...);
function array_multisort(&$arr1, ...);
function array_pad($input, $pad_size, $pad_value);
function array_pop(&$array);
function array_push(&$array, $var, ...);
function array_rand($input, $num_req = 1);
function array_reduce($input, $callback, $initial = null);
function array_reverse($array, $preserve_keys = false);
function array_search($needle, $haystack, $strict = false);
function array_shift(&$array);
function array_slice($array, $offset, $length = null, $preserve_keys = false);
function array_splice(&$input, $offset, $length = null, $replacement = null);
function array_unique($array, $sort_flags = 2);
function array_unshift(&$array, $var, ...);
function array_values<Tv>(Container<Tv> $input): array<Tv>;
function array_walk_recursive(&$input, $funcname, $userdata = null);
function array_walk(&$input, $funcname, $userdata = null);
function compact($varname, ...);
function shuffle(&$array);
<<__Deprecated('Use count(), it does the same thing as sizeof() in PHP and '.
  'doesn\'t suggest that it\'s counting bytes.')>>
function sizeof($var, $recursive = false);
function each(&$array);
function current(&$array);
function hphp_current_ref(&$array);
function next(&$array);
function pos(&$array);
function prev(&$array);
function reset(&$array);
function end(&$array);
function key(&$array);
function hphp_get_iterator($iterable);
function hphp_get_mutable_iterator(&$iterable);
function in_array($needle, $haystack, $strict = false);
function range($low, $high, $step = 1);
function array_diff($array1, $array2, ...);
function array_udiff($array1, $array2, $data_compare_func, ...);
function array_diff_assoc($array1, $array2, ...);
function array_diff_uassoc($array1, $array2, $key_compare_func, ...);
function array_udiff_assoc($array1, $array2, $data_compare_func, ...);
function array_udiff_uassoc($array1, $array2, $data_compare_func, $key_compare_func, ...);
function array_diff_key($array1, $array2, ...);
function array_diff_ukey($array1, $array2, $key_compare_func, ...);
function array_intersect($array1, $array2, ...);
function array_uintersect($array1, $array2, $data_compare_func, ...);
function array_intersect_assoc($array1, $array2, ...);
function array_intersect_uassoc($array1, $array2, $key_compare_func, ...);
function array_uintersect_assoc($array1, $array2, $data_compare_func, ...);
function array_uintersect_uassoc($array1, $array2, $data_compare_func, $key_compare_func, ...);
function array_intersect_key($array1, $array2, ...);
function array_intersect_ukey($array1, $array2, $key_compare_func, ...);
function natsort(&$array);
function natcasesort(&$array);
function i18n_loc_get_default();
function i18n_loc_set_default($locale);
function i18n_loc_set_attribute($attr, $val);
function i18n_loc_set_strength($strength);
function i18n_loc_get_error_code();
