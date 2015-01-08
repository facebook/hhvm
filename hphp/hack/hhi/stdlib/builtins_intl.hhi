<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function intl_get_error_code() { }
function intl_get_error_message() { }
function intl_error_name($error_code) { }
function intl_is_failure($error_code) { }
function collator_asort($obj, &$arr, $sort_flag = null) { }
function collator_compare($obj, $str1, $str2) { }
function collator_create($locale) { }
function collator_get_attribute($obj, $attr) { }
function collator_get_error_code($obj) { }
function collator_get_error_message($obj) { }
function collator_get_locale($obj, $type = 0) { }
function collator_get_strength($obj) { }
function collator_set_attribute($obj, $attr, $val) { }
function collator_set_strength($obj, $strength) { }
function collator_sort_with_sort_keys($obj, &$arr) { }
function collator_sort($obj, &$arr, $sort_flag = null) { }
function idn_to_ascii($domain, $options = 0, $variant = 0, &$idna_info = null) { }
function idn_to_unicode($domain, $options = 0, $variant = 0, &$idna_info = null) { }
function idn_to_utf8($domain, $options = 0, $variant = 0, &$idna_info = null) { }
class Collator {
  const SORT_REGULAR = 0;
  const SORT_NUMERIC = 0;
  const SORT_STRING = 0;
  const FRENCH_COLLATION = 0;
  const ALTERNATE_HANDLING = 0;
  const CASE_FIRST = 0;
  const CASE_LEVEL = 0;
  const NORMALIZATION_MODE = 0;
  const STRENGTH = 0;
  const HIRAGANA_QUATERNARY_MODE = 0;
  const NUMERIC_COLLATION = 0;
  const DEFAULT_VALUE = 0;
  const PRIMARY = 0;
  const SECONDARY = 0;
  const TERTIARY = 0;
  const DEFAULT_STRENGTH = 0;
  const QUATERNARY = 0;
  const IDENTICAL = 0;
  const OFF = 0;
  const ON = 0;
  const SHIFTED = 0;
  const NON_IGNORABLE = 0;
  const LOWER_FIRST = 0;
  const UPPER_FIRST = 0;
  public function __construct($locale) { }
  public function asort(&$arr, $sort_flag = null) { }
  public function compare($str1, $str2) { }
  static public function create($locale) { }
  public function getattribute($attr) { }
  public function geterrorcode() { }
  public function geterrormessage() { }
  public function getlocale($type = 0) { }
  public function getstrength() { }
  public function setattribute($attr, $val) { }
  public function setstrength($strength) { }
  public function sortwithsortkeys(&$arr) { }
  public function sort(&$arr, $sort_flag = null) { }
}
class Locale {
  const ACTUAL_LOCALE = 0;
  const VALID_LOCALE = 0;
  public function __construct() { }
}
class Normalizer {
  const NONE = 0;
  const FORM_D = 0;
  const NFD = 0;
  const FORM_KD = 0;
  const NFKD = 0;
  const FORM_C = 0;
  const NFC = 0;
  const FORM_KC = 0;
  const NFKC = 0;
  public function __construct() { }
  static public function isnormalized($input, $form = null) { }
  static public function normalize($input, $form = null) { }
}
