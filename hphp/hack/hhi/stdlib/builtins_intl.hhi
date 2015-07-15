<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const int INTL_MAX_LOCALE_LEN = 80;
const string INTL_ICU_VERSION = '52.1';
const string INTL_ICU_DATA_VERSION = '52.1';

const int IDNA_ALLOW_UNASSIGNED = 1;
const int IDNA_CHECK_BIDI = 4;
const int IDNA_CHECK_CONTEXTJ = 8;
const int IDNA_CHECK_CONTEXTO = 64;
const int IDNA_CONTAINS_ACE_PREFIX = 8;
const int IDNA_CONTAINS_MINUS = 4;
const int IDNA_CONTAINS_NON_LDH = 3;
const int IDNA_DEFAULT = 0;
const int IDNA_ERROR_CONTEXTJ = 4096;
const int IDNA_ERROR_CONTEXTO_DIGITS = 8192;
const int IDNA_ERROR_CONTEXTO_PUNCTUATION = 16384;
const int IDNA_ERROR_BIDI = 2048;
const int IDNA_ERROR_DISALLOWED = 128;
const int IDNA_ERROR_DOMAIN_NAME_TOO_LONG = 4;
const int IDNA_ERROR_EMPTY_LABEL = 1;
const int IDNA_ERROR_HYPHEN_3_4 = 32;
const int IDNA_ERROR_INVALID_ACE_LABEL = 1024;
const int IDNA_ERROR_LABEL_HAS_DOT = 512;
const int IDNA_ERROR_LABEL_TOO_LONG = 2;
const int IDNA_ERROR_LEADING_COMBINING_MARK = 64;
const int IDNA_ERROR_LEADING_HYPHEN = 8;
const int IDNA_ERROR_PUNYCODE = 256;
const int IDNA_ERROR_TRAILING_HYPHEN = 16;
const int IDNA_ICONV_ERROR = 9;
const int IDNA_INVALID_LENGTH = 5;
const int IDNA_MALLOC_ERROR = 201;
const int IDNA_NO_ACE_PREFIX = 6;
const int IDNA_NONTRANSITIONAL_TO_ASCII = 16;
const int IDNA_NONTRANSITIONAL_TO_UNICODE = 32;
const int IDNA_PUNYCODE_ERROR = 2;
const int IDNA_ROUNDTRIP_VERIFY_ERROR = 7;
const int IDNA_STRINGPREP_ERROR = 1;
const int IDNA_USE_STD3_RULES = 2;

const int U_AMBIGUOUS_ALIAS_WARNING = -122;
const int U_BAD_VARIABLE_DEFINITION = 65536;
const int U_BRK_ASSIGN_ERROR = 66053;
const int U_BRK_ERROR_LIMIT = 66062;
const int U_BRK_ERROR_START = 66048;
const int U_BRK_HEX_DIGITS_EXPECTED = 66049;
const int U_BRK_INIT_ERROR = 66058;
const int U_BRK_INTERNAL_ERROR = 66048;
const int U_BRK_MALFORMED_RULE_TAG = 66061;
const int U_BRK_MISMATCHED_PAREN = 66055;
const int U_BRK_NEW_LINE_IN_QUOTED_STRING = 66056;
const int U_BRK_RULE_EMPTY_SET = 66059;
const int U_BRK_RULE_SYNTAX = 66051;
const int U_BRK_SEMICOLON_EXPECTED = 66050;
const int U_BRK_UNCLOSED_SET = 66052;
const int U_BRK_UNDEFINED_VARIABLE = 66057;
const int U_BRK_UNRECOGNIZED_OPTION = 66060;
const int U_BRK_VARIABLE_REDFINITION = 66054;
const int U_BUFFER_OVERFLOW_ERROR = 15;
const int U_CE_NOT_FOUND_ERROR = 21;
const int U_COLLATOR_VERSION_MISMATCH = 28;
const int U_DIFFERENT_UCA_VERSION = -121;
const int U_ENUM_OUT_OF_SYNC_ERROR = 25;
const int U_ERROR_LIMIT = 66568;
const int U_ERROR_WARNING_LIMIT = -120;
const int U_ERROR_WARNING_START = -128;
const int U_FILE_ACCESS_ERROR = 4;
const int U_FMT_PARSE_ERROR_LIMIT = 65804;
const int U_FMT_PARSE_ERROR_START = 65792;
const int U_ILLEGAL_ARGUMENT_ERROR = 1;
const int U_ILLEGAL_CHARACTER = 65567;
const int U_ILLEGAL_CHAR_FOUND = 12;
const int U_ILLEGAL_CHAR_IN_SEGMENT = 65564;
const int U_ILLEGAL_ESCAPE_SEQUENCE = 18;
const int U_ILLEGAL_PAD_POSITION = 65800;
const int U_INDEX_OUTOFBOUNDS_ERROR = 8;
const int U_INTERNAL_PROGRAM_ERROR = 5;
const int U_INTERNAL_TRANSLITERATOR_ERROR = 65568;
const int U_INVALID_CHAR_FOUND = 10;
const int U_INVALID_FORMAT_ERROR = 3;
const int U_INVALID_FUNCTION = 65570;
const int U_INVALID_ID = 65569;
const int U_INVALID_PROPERTY_PATTERN = 65561;
const int U_INVALID_RBT_SYNTAX = 65560;
const int U_INVALID_STATE_ERROR = 27;
const int U_INVALID_TABLE_FILE = 14;
const int U_INVALID_TABLE_FORMAT = 13;
const int U_INVARIANT_CONVERSION_ERROR = 26;
const int U_MALFORMED_EXPONENTIAL_PATTERN = 65795;
const int U_MALFORMED_PRAGMA = 65562;
const int U_MALFORMED_RULE = 65537;
const int U_MALFORMED_SET = 65538;
const int U_MALFORMED_SYMBOL_REFERENCE = 65539;
const int U_MALFORMED_UNICODE_ESCAPE = 65540;
const int U_MALFORMED_VARIABLE_DEFINITION = 65541;
const int U_MALFORMED_VARIABLE_REFERENCE = 65542;
const int U_MEMORY_ALLOCATION_ERROR = 7;
const int U_MESSAGE_PARSE_ERROR = 6;
const int U_MISMATCHED_SEGMENT_DELIMITERS = 65543;
const int U_MISPLACED_ANCHOR_START = 65544;
const int U_MISPLACED_COMPOUND_FILTER = 65558;
const int U_MISPLACED_CURSOR_OFFSET = 65545;
const int U_MISPLACED_QUANTIFIER = 65546;
const int U_MISSING_OPERATOR = 65547;
const int U_MISSING_RESOURCE_ERROR = 2;
const int U_MISSING_SEGMENT_CLOSE = 65548;
const int U_MULTIPLE_ANTE_CONTEXTS = 65549;
const int U_MULTIPLE_COMPOUND_FILTERS = 65559;
const int U_MULTIPLE_CURSORS = 65550;
const int U_MULTIPLE_EXPONENTIAL_SYMBOLS = 65794;
const int U_MULTIPLE_PAD_SPECIFIERS = 65798;
const int U_MULTIPLE_PERCENT_SYMBOLS = 65796;
const int U_MULTIPLE_PERMILL_SYMBOLS = 65797;
const int U_MULTIPLE_POST_CONTEXTS = 65551;
const int U_NO_SPACE_AVAILABLE = 20;
const int U_NO_WRITE_PERMISSION = 30;
const int U_PARSE_ERROR = 9;
const int U_PARSE_ERROR_LIMIT = 65571;
const int U_PARSE_ERROR_START = 65536;
const int U_PATTERN_SYNTAX_ERROR = 65799;
const int U_PRIMARY_TOO_LONG_ERROR = 22;
const int U_REGEX_BAD_ESCAPE_SEQUENCE = 66307;
const int U_REGEX_BAD_INTERVAL = 66312;
const int U_REGEX_ERROR_LIMIT = 66318;
const int U_REGEX_ERROR_START = 66304;
const int U_REGEX_INTERNAL_ERROR = 66304;
const int U_REGEX_INVALID_BACK_REF = 66314;
const int U_REGEX_INVALID_FLAG = 66315;
const int U_REGEX_INVALID_STATE = 66306;
const int U_REGEX_LOOK_BEHIND_LIMIT = 66316;
const int U_REGEX_MAX_LT_MIN = 66313;
const int U_REGEX_MISMATCHED_PAREN = 66310;
const int U_REGEX_NUMBER_TOO_BIG = 66311;
const int U_REGEX_PROPERTY_SYNTAX = 66308;
const int U_REGEX_RULE_SYNTAX = 66305;
const int U_REGEX_SET_CONTAINS_STRING = 66317;
const int U_REGEX_UNIMPLEMENTED = 66309;
const int U_RESOURCE_TYPE_MISMATCH = 17;
const int U_RULE_MASK_ERROR = 65557;
const int U_SAFECLONE_ALLOCATED_WARNING = -126;
const int U_SORT_KEY_TOO_SHORT_WARNING = -123;
const int U_STANDARD_ERROR_LIMIT = 31;
const int U_STATE_OLD_WARNING = -125;
const int U_STATE_TOO_OLD_ERROR = 23;
const int U_STRINGPREP_CHECK_BIDI_ERROR = 66562;
const int U_STRINGPREP_PROHIBITED_ERROR = 66560;
const int U_STRINGPREP_UNASSIGNED_ERROR = 66561;
const int U_STRING_NOT_TERMINATED_WARNING = -124;
const int U_TOO_MANY_ALIASES_ERROR = 24;
const int U_TRAILING_BACKSLASH = 65552;
const int U_TRUNCATED_CHAR_FOUND = 11;
const int U_UNCLOSED_SEGMENT = 65563;
const int U_UNDEFINED_SEGMENT_REFERENCE = 65553;
const int U_UNDEFINED_VARIABLE = 65554;
const int U_UNEXPECTED_TOKEN = 65792;
const int U_UNMATCHED_BRACES = 65801;
const int U_UNQUOTED_SPECIAL = 65555;
const int U_UNSUPPORTED_ATTRIBUTE = 65803;
const int U_UNSUPPORTED_ERROR = 16;
const int U_UNSUPPORTED_ESCAPE_SEQUENCE = 19;
const int U_UNSUPPORTED_PROPERTY = 65802;
const int U_UNTERMINATED_QUOTE = 65556;
const int U_USELESS_COLLATOR_ERROR = 29;
const int U_USING_DEFAULT_WARNING = -127;
const int U_USING_FALLBACK_WARNING = -128;
const int U_VARIABLE_RANGE_EXHAUSTED = 65565;
const int U_VARIABLE_RANGE_OVERLAP = 65566;
const int U_ZERO_ERROR = 0;

function intl_get_error_code() {}
function intl_get_error_message() {}
function intl_error_name($error_code) {}
function intl_is_failure($error_code) {}
function collator_asort($obj, &$arr, $sort_flag = null) {}
function collator_compare($obj, $str1, $str2) {}
function collator_create($locale) {}
function collator_get_attribute($obj, $attr) {}
function collator_get_error_code($obj) {}
function collator_get_error_message($obj) {}
function collator_get_locale($obj, $type = 0) {}
function collator_get_strength($obj) {}
function collator_set_attribute($obj, $attr, $val) {}
function collator_set_strength($obj, $strength) {}
function collator_sort_with_sort_keys($obj, &$arr) {}
function collator_sort($obj, &$arr, $sort_flag = null) {}
function idn_to_ascii($domain, $options = 0, $variant = 0, &$idna_info = null) {}
function idn_to_unicode($domain, $options = 0, $variant = 0, &$idna_info = null) {}
function idn_to_utf8($domain, $options = 0, $variant = 0, &$idna_info = null) {}
function datefmt_create($locale, $date_type, $time_type, $timezone_str = null, $calendar = null, $pattern = null) {}
function datefmt_format($args = null, $array = null) {}
function datefmt_format_object($object, $format = null, $locale = null) {}
function datefmt_get_calendar($mf) {}
function datefmt_get_calendar_object($mf) {}
function datefmt_get_datetype($mf) {}
function datefmt_get_error_code($nf) {}
function datefmt_get_error_message($coll) {}
function datefmt_get_locale($mf) {}
function datefmt_get_pattern($mf) {}
function datefmt_get_timetype($mf) {}
function datefmt_get_timezone($mf) {}
function datefmt_get_timezone_id($mf) {}
function datefmt_is_lenient($mf) {}
function datefmt_localtime($formatter, $string, &$position = null) {}
function datefmt_parse($formatter, $string, &$position = null) {}
function datefmt_set_calendar($mf, $calendar) {}
function datefmt_set_lenient($mf) {}
function datefmt_set_pattern($mf, $pattern) {}
function datefmt_set_timezone($mf, $timezone) {}
function datefmt_set_timezone_id($mf, $timezone) {}
function grapheme_extract($arg1, $arg2, $arg3 = null, $arg4 = null, &$arg5 = null) {}
function grapheme_stripos($haystack, $needle, $offset = null) {}
function grapheme_stristr($haystack, $needle, $before_needle = null) {}
function grapheme_strlen($string) {}
function grapheme_strpos($haystack, $needle, $offset = null) {}
function grapheme_strripos($haystack, $needle, $offset = null) {}
function grapheme_strrpos($haystack, $needle, $offset = null) {}
function grapheme_strstr($haystack, $needle, $before_needle = null) {}
function grapheme_substr($string, $start, $length = null) {}
function intlcal_add(IntlCalendar $calendar, $field, $amount) {}
function intlcal_after(IntlCalendar $calendar, IntlCalendar $otherCalendar) {}
function intlcal_before(IntlCalendar $calendar, IntlCalendar $otherCalendar) {}
function intlcal_clear(IntlCalendar $calendar, $field = null) {}
function intlcal_create_instance($timeZone = null, $locale = null) {}
function intlcal_equals(IntlCalendar $calendar, IntlCalendar $otherCalendar) {}
function intlcal_field_difference(IntlCalendar $calendar, $when, $field) {}
function intlcal_from_date_time($dateTime) {}
function intlcal_get(IntlCalendar $calendar, $field) {}
function intlcal_get_actual_maximum(IntlCalendar $calendar, $field) {}
function intlcal_get_actual_minimum(IntlCalendar $calendar, $field) {}
function intlcal_get_available_locales() {}
function intlcal_get_day_of_week_type(IntlCalendar $calendar, $dayOfWeek) {}
function intlcal_get_error_code(IntlCalendar $calendar) {}
function intlcal_get_error_message(IntlCalendar $calendar) {}
function intlcal_get_first_day_of_week(IntlCalendar $calendar) {}
function intlcal_get_greatest_minimum(IntlCalendar $calendar, $field) {}
function intlcal_get_keyword_values_for_locale($key, $locale, $commonlyUsed) {}
function intlcal_get_least_maximum(IntlCalendar $calendar, $field) {}
function intlcal_get_locale(IntlCalendar $calendar, $localeType) {}
function intlcal_get_maximum(IntlCalendar $calendar, $field) {}
function intlcal_get_minimal_days_in_first_week(IntlCalendar $calendar) {}
function intlcal_get_minimum(IntlCalendar $calendar, $field) {}
function intlcal_get_now() {}
function intlcal_get_repeated_wall_time_option(IntlCalendar $calendar) {}
function intlcal_get_skipped_wall_time_option(IntlCalendar $calendar) {}
function intlcal_get_time(IntlCalendar $calendar) {}
function intlcal_get_time_zone(IntlCalendar $calendar) {}
function intlcal_get_type(IntlCalendar $calendar) {}
function intlcal_get_weekend_transition(IntlCalendar $calendar, $dayOfWeek) {}
function intlcal_in_daylight_time(IntlCalendar $calendar) {}
function intlcal_is_equivalent_to(IntlCalendar $calendar, IntlCalendar $otherCalendar) {}
function intlcal_is_lenient(IntlCalendar $calendar) {}
function intlcal_is_set(IntlCalendar $calendar, $field) {}
function intlcal_is_weekend(IntlCalendar $calendar, $date = null) {}
function intlcal_roll(IntlCalendar $calendar, $field, $amountOrUpOrDown = null) {}
function intlcal_set(IntlCalendar $calendar, $fieldOrYear, $valueOrMonth, $dayOfMonth = null, $hour = null, $minute = null, $second = null) {}
function intlcal_set_first_day_of_week(IntlCalendar $calendar, $dayOfWeek) {}
function intlcal_set_lenient(IntlCalendar $calendar, $isLenient) {}
function intlcal_set_minimal_days_in_first_week(IntlCalendar $calendar, $numberOfDays) {}
function intlcal_set_repeated_wall_time_option(IntlCalendar $calendar, $wallTimeOption) {}
function intlcal_set_skipped_wall_time_option(IntlCalendar $calendar, $wallTimeOption) {}
function intlcal_set_time(IntlCalendar $calendar, $date) {}
function intlcal_set_time_zone(IntlCalendar $calendar, $timeZone) {}
function intlcal_to_date_time(IntlCalendar $calendar) {}
function intlgregcal_create_instance($timeZoneOrYear = null, $localeOrMonth = null, $dayOfMonth = null, $hour = null, $minute = null, $second = null) {}
function intlgregcal_get_gregorian_change(IntlGregorianCalendar $calendar) {}
function intlgregcal_is_leap_year(IntlGregorianCalendar $calendar, $year) {}
function intlgregcal_set_gregorian_change(IntlGregorianCalendar $calendar, $date) {}
function intltz_count_equivalent_ids($zoneId) {}
function intltz_create_default() {}
function intltz_create_enumeration($countryOrRawOffset = null) {}
function intltz_create_time_zone($zoneId) {}
function intltz_create_time_zone_id_enumeration($zoneType, $region = null, $rawOffset = null) {}
function intltz_from_date_time_zone(DateTimeZone $dateTimeZone) {}
function intltz_get_canonical_id($zoneId, &$isSystemID = null) {}
function intltz_get_display_name(IntlTimeZone $timeZone, $isDaylight = null, $style = null, $locale = null) {}
function intltz_get_dst_savings(IntlTimeZone $timeZone) {}
function intltz_get_equivalent_id($zoneId, $index) {}
function intltz_get_error_code(IntlTimeZone $timeZone) {}
function intltz_get_error_message(IntlTimeZone $timeZone) {}
function intltz_get_gmt() {}
function intltz_get_id(IntlTimeZone $timeZone) {}
function intltz_get_offset(IntlTimeZone $timeZone, $date, $local, &$rawOffset, &$dstOffset) {}
function intltz_get_raw_offset(IntlTimeZone $timeZone) {}
function intltz_get_region($zoneId) {}
function intltz_get_tz_data_version() {}
function intltz_get_unknown() {}
function intltz_has_same_rules(IntlTimeZone $timeZone, ?IntlTimeZone $otherTimeZone = null) {}
function intltz_to_date_time_zone(IntlTimeZone $timeZone) {}
function intltz_use_daylight_time(IntlTimeZone $timeZone) {}
function locale_accept_from_http($arg1) {}
function locale_canonicalize($arg1) {}
function locale_compose($arg1) {}
function locale_filter_matches($arg1, $arg2, $arg3) {}
function locale_get_all_variants($arg1) {}
function locale_get_default() {}
function locale_get_display_language($arg1, $arg2) {}
function locale_get_display_name($arg1, $arg2) {}
function locale_get_display_region($arg1, $arg2) {}
function locale_get_display_script($arg1, $arg2) {}
function locale_get_display_variant($arg1, $arg2) {}
function locale_get_keywords($arg1) {}
function locale_get_primary_language($arg1) {}
function locale_get_region($arg1) {}
function locale_get_script($arg1) {}
function locale_lookup($arg1, $arg2, $arg3, $arg4) {}
function locale_parse($arg1) {}
function locale_set_default($arg1) {}
function msgfmt_create($locale, $pattern) {}
function msgfmt_format($nf, $args) {}
function msgfmt_format_message($locale, $pattern, $args) {}
function msgfmt_get_error_code($nf) {}
function msgfmt_get_error_message($coll) {}
function msgfmt_get_locale($mf) {}
function msgfmt_get_pattern($mf) {}
function msgfmt_parse($nf, $source) {}
function msgfmt_parse_message($locale, $pattern, $source) {}
function msgfmt_set_pattern($mf, $pattern) {}
function normalizer_is_normalized($input, $form = null) {}
function normalizer_normalize($input, $form = null) {}
function numfmt_create($locale, $style, $pattern = null) {}
function numfmt_format($nf, $num, $type = null) {}
function numfmt_format_currency($nf, $num, $currency) {}
function numfmt_get_attribute($nf, $attr) {}
function numfmt_get_error_code($nf) {}
function numfmt_get_error_message($nf) {}
function numfmt_get_locale($nf, $type = null) {}
function numfmt_get_pattern($nf) {}
function numfmt_get_symbol($nf, $attr) {}
function numfmt_get_text_attribute($nf, $attr) {}
function numfmt_parse($formatter, $string, $type = null, &$position = null) {}
function numfmt_parse_currency($formatter, $string, &$currency, &$position = null) {}
function numfmt_set_attribute($nf, $attr, $value) {}
function numfmt_set_pattern($nf, $pattern) {}
function numfmt_set_symbol($nf, $attr, $symbol) {}
function numfmt_set_text_attribute($nf, $attr, $value) {}
function resourcebundle_count($bundle) {}
function resourcebundle_create($locale, $bundlename, $fallback = null) {}
function resourcebundle_get($bundle, $index, $fallback = null) {}
function resourcebundle_get_error_code($bundle) {}
function resourcebundle_get_error_message($bundle) {}
function resourcebundle_locales($bundlename) {}

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
  public function __construct($locale) {}
  public function asort(&$arr, $sort_flag = null) {}
  public function compare($str1, $str2) {}
  static public function create($locale) {}
  public function getattribute($attr) {}
  public function geterrorcode() {}
  public function geterrormessage() {}
  public function getlocale($type = 0) {}
  public function getstrength() {}
  public function setattribute($attr, $val) {}
  public function setstrength($strength) {}
  public function sortwithsortkeys(&$arr) {}
  public function sort(&$arr, $sort_flag = null) {}
}

class Locale {
  const int ACTUAL_LOCALE = 0;
  const int VALID_LOCALE = 1;
  const string DEFAULT_LOCALE = '';
  const string LANG_TAG = 'language';
  const string EXTLANG_TAG = 'extlang';
  const string SCRIPT_TAG = 'script';
  const string REGION_TAG = 'region';
  const string VARIANT_TAG = 'variant';
  const string GRANDFATHERED_LANG_TAG = 'grandfathered';
  const string PRIVATE_TAG = 'private';
  public static function acceptFromHttp($arg1) {}
  public static function canonicalize($arg1) {}
  public static function composeLocale($arg1) {}
  public static function filterMatches($arg1, $arg2, $arg3) {}
  public static function getAllVariants($arg1) {}
  public static function getDefault() {}
  public static function getDisplayLanguage($arg1, $arg2) {}
  public static function getDisplayName($arg1, $arg2) {}
  public static function getDisplayRegion($arg1, $arg2) {}
  public static function getDisplayScript($arg1, $arg2) {}
  public static function getDisplayVariant($arg1, $arg2) {}
  public static function getKeywords($arg1) {}
  public static function getPrimaryLanguage($arg1) {}
  public static function getRegion($arg1) {}
  public static function getScript($arg1) {}
  public static function lookup($arg1, $arg2, $arg3, $arg4) {}
  public static function parseLocale($arg1) {}
  public static function setDefault($arg1) {}
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
  public function __construct() {}
  static public function isNormalized($input, $form = null) {}
  static public function normalize($input, $form = null) {}
}

class MessageFormatter {
  public function __construct($locale, $pattern) {}
  public static function create($locale, $pattern) {}
  public function format($args) {}
  public static function formatMessage($locale, $pattern, $args) {}
  public function getErrorCode() {}
  public function getErrorMessage() {}
  public function getLocale() {}
  public function getPattern() {}
  public function parse($source) {}
  public static function parseMessage($locale, $pattern, $args) {}
  public function setPattern($pattern) {}
}

class IntlDateFormatter {
  const int FULL = 0;
  const int LONG = 1;
  const int MEDIUM = 2;
  const int SHORT = 3;
  const int NONE = -1;
  const int GREGORIAN = 1;
  const int TRADITIONAL = 0;
  public function __construct($locale, $datetype, $timetype, $timezone = null, $calendar = null, $pattern = null) {}
  public static function create($locale, $datetype, $timetype, $timezone = null, $calendar = null, $pattern = null) {}
  public function format($args = null, $array = null) {}
  public static function formatObject($object, $format = null, $locale = null) {}
  public function getCalendar() {}
  public function getCalendarObject() {}
  public function getDateType() {}
  public function getErrorCode() {}
  public function getErrorMessage() {}
  public function getLocale() {}
  public function getPattern() {}
  public function getTimeType() {}
  public function getTimeZone() {}
  public function getTimeZoneId() {}
  public function isLenient() {}
  public function localtime($string, &$position = null) {}
  public function parse($string, &$position = null) {}
  public function setCalendar($which) {}
  public function setLenient($lenient) {}
  public function setPattern($pattern) {}
  public function setTimeZone($zone) {}
  public function setTimeZoneId($zone) {}
}

class ResourceBundle<T> implements Traversable<T> {
  public function __construct($locale, $bundlename, $fallback = null) {}
  public function count() {}
  public static function create($locale, $bundlename, $fallback = null) {}
  public function get($index, $fallback = null) {}
  public function getErrorCode() {}
  public function getErrorMessage() {}
  public static function getLocales($bundlename) {}
}

class IntlTimeZone {
  const int DISPLAY_SHORT = 1;
  const int DISPLAY_LONG = 2;
  const int DISPLAY_SHORT_GENERIC = 3;
  const int DISPLAY_LONG_GENERIC = 4;
  const int DISPLAY_SHORT_GMT = 5;
  const int DISPLAY_LONG_GMT = 6;
  const int DISPLAY_SHORT_COMMONLY_USED = 7;
  const int DISPLAY_GENERIC_LOCATION = 8;
  const int TYPE_ANY = 0;
  const int TYPE_CANONICAL = 1;
  const int TYPE_CANONICAL_LOCATION = 2;
  private function __construct() {}
  public static function countEquivalentIDs($zoneId) {}
  public static function createDefault() {}
  public static function createEnumeration($countryOrRawOffset = null) {}
  public static function createTimeZone($zoneId) {}
  public static function createTimeZoneIDEnumeration($zoneType, $region = null, $rawOffset = null) {}
  public static function fromDateTimeZone($zoneId) {}
  public static function getCanonicalID($zoneId, &$isSystemID = null) {}
  public function getDSTSavings() {}
  public function getDisplayName($isDaylight = null, $style = null, $locale = null) {}
  public static function getEquivalentID($zoneId, $index) {}
  public function getErrorCode() {}
  public function getErrorMessage() {}
  public static function getGMT() {}
  public function getID() {}
  public function getOffset($date, $local, &$rawOffset, &$dstOffset) {}
  public function getRawOffset() {}
  public static function getRegion($zoneId) {}
  public static function getTZDataVersion() {}
  public static function getUnknown() {}
  public function hasSameRules(IntlTimeZone $otherTimeZone) {}
  public function toDateTimeZone() {}
  public function useDaylightTime() {}
}

class IntlCalendar {
  const int FIELD_ERA = 0;
  const int FIELD_YEAR = 1;
  const int FIELD_MONTH = 2;
  const int FIELD_WEEK_OF_YEAR = 3;
  const int FIELD_WEEK_OF_MONTH = 4;
  const int FIELD_DATE = 5;
  const int FIELD_DAY_OF_YEAR = 6;
  const int FIELD_DAY_OF_WEEK = 7;
  const int FIELD_DAY_OF_WEEK_IN_MONTH = 8;
  const int FIELD_AM_PM = 9;
  const int FIELD_HOUR = 10;
  const int FIELD_HOUR_OF_DAY = 11;
  const int FIELD_MINUTE = 12;
  const int FIELD_SECOND = 13;
  const int FIELD_MILLISECOND = 14;
  const int FIELD_ZONE_OFFSET = 15;
  const int FIELD_DST_OFFSET = 16;
  const int FIELD_YEAR_WOY = 17;
  const int FIELD_DOW_LOCAL = 18;
  const int FIELD_EXTENDED_YEAR = 19;
  const int FIELD_JULIAN_DAY = 20;
  const int FIELD_MILLISECONDS_IN_DAY = 21;
  const int FIELD_IS_LEAP_MONTH = 22;
  const int FIELD_FIELD_COUNT = 23;
  const int FIELD_DAY_OF_MONTH = 5;
  const int DOW_SUNDAY = 1;
  const int DOW_MONDAY = 2;
  const int DOW_TUESDAY = 3;
  const int DOW_WEDNESDAY = 4;
  const int DOW_THURSDAY = 5;
  const int DOW_FRIDAY = 6;
  const int DOW_SATURDAY = 7;
  const int DOW_TYPE_WEEKDAY = 0;
  const int DOW_TYPE_WEEKEND = 1;
  const int DOW_TYPE_WEEKEND_OFFSET = 2;
  const int DOW_TYPE_WEEKEND_CEASE = 3;
  const int WALLTIME_FIRST = 1;
  const int WALLTIME_LAST = 0;
  const int WALLTIME_NEXT_VALID = 2;
  private function __construct() {}
  public function add($field, $amount) {}
  public function after(IntlCalendar $calendar) {}
  public function before(IntlCalendar $calendar) {}
  public function clear($field = null) {}
  public static function createInstance($timeZone = null, $locale = null) {}
  public function equals(IntlCalendar $calendar) {}
  public function fieldDifference($when, $field) {}
  public static function fromDateTime($dateTime) {}
  public function get($field) {}
  public function getActualMaximum($field) {}
  public function getActualMinimum($field) {}
  public static function getAvailableLocales() {}
  public function getDayOfWeekType($dayOfWeek) {}
  public function getErrorCode() {}
  public function getErrorMessage() {}
  public function getFirstDayOfWeek() {}
  public function getGreatestMinimum($field) {}
  public static function getKeywordValuesForLocale($key, $locale, $commonlyUsed) {}
  public function getLeastMaximum($field) {}
  public function getLocale($localeType) {}
  public function getMaximum($field) {}
  public function getMinimalDaysInFirstWeek() {}
  public function getMinimum($field) {}
  public static function getNow() {}
  public function getRepeatedWallTimeOption() {}
  public function getSkippedWallTimeOption() {}
  public function getTime() {}
  public function getTimeZone() {}
  public function getType() {}
  public function getWeekendTransition($dayOfWeek) {}
  public function inDaylightTime() {}
  public function isEquivalentTo(IntlCalendar $calendar) {}
  public function isLenient() {}
  public function isSet($field) {}
  public function isWeekend($date = null) {}
  public function roll($field, $amountOrUpOrDown) {}
  public function set($fieldOrYear, $valueOrMonth, $dayOfMonth = null, $hour = null, $minute = null, $second = null) {}
  public function setFirstDayOfWeek($dayOfWeek) {}
  public function setLenient($isLenient) {}
  public function setMinimalDaysInFirstWeek($numberOfDays) {}
  public function setRepeatedWallTimeOption($wallTimeOption) {}
  public function setSkippedWallTimeOption($wallTimeOption) {}
  public function setTime($date) {}
  public function setTimeZone($timeZone) {}
  public function toDateTime() {}
}

class IntlGregorianCalendar extends IntlCalendar {
  public function getGregorianChange() {}
  public function isLeapYear($year) {}
  public function setGregorianChange($date) {}
}

class IntlIterator<Tv> implements KeyedIterator<int, Tv> {
  // Methods
  public function current(): Tv;
  public function key(): int;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;

}

class IntlBreakIterator implements KeyedTraversable<int, int> {

  // Constants
  const DONE = -1;
  const WORD_NONE = 0;
  const WORD_NONE_LIMIT = 100;
  const WORD_NUMBER = 100;
  const WORD_LETTER = 200;
  const WORD_KANA = 300;
  const WORD_KANA_LIMIT = 400;
  const WORD_IDEO = 400;
  const WORD_IDEO_LIMIT = 500;
  const LINE_SOFT = 0;
  const LINE_SOFT_LIMIT = 100;
  const LINE_HARD = 100;
  const LINE_HARD_LIMIT = 200;
  const SENTENCE_TERM = 0;
  const SENTENCE_SEP = 100;
  const WORD_NUMBER_LIMIT = 200;
  const WORD_LETTER_LIMIT = 300;
  const SENTENCE_TERM_LIMIT = 100;
  const SENTENCE_SEP_LIMIT = 200;

  // Methods
  public static function createCharacterInstance(
    $locale = null,
  ): IntlBreakIterator;
  public static function createCodePointInstance(
  ): IntlCodePointBreakIterator;
  public static function createLineInstance(
    $locale = null,
  ): IntlBreakIterator;
  public static function createSentenceInstance(
    $locale = null,
  ): IntlBreakIterator;
  public static function createTitleInstance(
    $locale = null,
  ): IntlBreakIterator;
  public static function createWordInstance(
    $locale = null,
  ): IntlBreakIterator;
  public function key(): mixed; // returns int or false
  public function rewind(): int;
  public function valid(): bool;
  public function current(): int;
  public function first(): int;
  public function following(int $offset): mixed; // returns int or false
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public function getLocale(int $locale_type): mixed; // returns string or false
  public function getPartsIterator(string $key_type): IntlPartsIterator;
  public function getText(): ?string;
  public function isBoundary(int $offset): bool;
  public function last(): int;
  public function next($offset = null): mixed; // returns int or false
  public function preceding(int $offset): mixed; // returns int or false
  public function previous(): int;
  public function setText(string $text): bool;
}

class IntlRuleBasedBreakIterator extends IntlBreakIterator {
  // Methods
  public function __construct(string $rules, bool $compiled = false);
  public function getRules(): mixed; // returns string or false
  public function getRuleStatus(): int;
  public function getRuleStatusVec(): mixed; // returns array<int> or false
  public function getBinaryRules(): mixed; // returns string or false
}

class IntlCodePointBreakIterator extends IntlBreakIterator {
  public function getLastCodePoint(): int {}
}

class IntlPartsIterator extends IntlIterator<string> {
  const int KEY_SEQUENTIAL = 0;
  const int KEY_LEFT = 1;
  const int KEY_RIGHT = 2;
  public function getBreakIterator(): IntlBreakIterator {}
}
