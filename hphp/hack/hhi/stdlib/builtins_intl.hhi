<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int INTL_MAX_LOCALE_LEN;
const string INTL_ICU_VERSION;
const string INTL_ICU_DATA_VERSION;

const int INTL_IDNA_VARIANT_2003;
const int INTL_IDNA_VARIANT_UTS46;

const int IDNA_ALLOW_UNASSIGNED;
const int IDNA_CHECK_BIDI;
const int IDNA_CHECK_CONTEXTJ;
const int IDNA_CHECK_CONTEXTO;
const int IDNA_CONTAINS_ACE_PREFIX;
const int IDNA_CONTAINS_MINUS;
const int IDNA_CONTAINS_NON_LDH;
const int IDNA_DEFAULT;
const int IDNA_ERROR_CONTEXTJ;
const int IDNA_ERROR_CONTEXTO_DIGITS;
const int IDNA_ERROR_CONTEXTO_PUNCTUATION;
const int IDNA_ERROR_BIDI;
const int IDNA_ERROR_DISALLOWED;
const int IDNA_ERROR_DOMAIN_NAME_TOO_LONG;
const int IDNA_ERROR_EMPTY_LABEL;
const int IDNA_ERROR_HYPHEN_3_4;
const int IDNA_ERROR_INVALID_ACE_LABEL;
const int IDNA_ERROR_LABEL_HAS_DOT;
const int IDNA_ERROR_LABEL_TOO_LONG;
const int IDNA_ERROR_LEADING_COMBINING_MARK;
const int IDNA_ERROR_LEADING_HYPHEN;
const int IDNA_ERROR_PUNYCODE;
const int IDNA_ERROR_TRAILING_HYPHEN;
const int IDNA_ICONV_ERROR;
const int IDNA_INVALID_LENGTH;
const int IDNA_MALLOC_ERROR;
const int IDNA_NO_ACE_PREFIX;
const int IDNA_NONTRANSITIONAL_TO_ASCII;
const int IDNA_NONTRANSITIONAL_TO_UNICODE;
const int IDNA_PUNYCODE_ERROR;
const int IDNA_ROUNDTRIP_VERIFY_ERROR;
const int IDNA_STRINGPREP_ERROR;
const int IDNA_USE_STD3_RULES;

const int U_AMBIGUOUS_ALIAS_WARNING;
const int U_BAD_VARIABLE_DEFINITION;
const int U_BRK_ASSIGN_ERROR;
const int U_BRK_ERROR_LIMIT;
const int U_BRK_ERROR_START;
const int U_BRK_HEX_DIGITS_EXPECTED;
const int U_BRK_INIT_ERROR;
const int U_BRK_INTERNAL_ERROR;
const int U_BRK_MALFORMED_RULE_TAG;
const int U_BRK_MISMATCHED_PAREN;
const int U_BRK_NEW_LINE_IN_QUOTED_STRING;
const int U_BRK_RULE_EMPTY_SET;
const int U_BRK_RULE_SYNTAX;
const int U_BRK_SEMICOLON_EXPECTED;
const int U_BRK_UNCLOSED_SET;
const int U_BRK_UNDEFINED_VARIABLE;
const int U_BRK_UNRECOGNIZED_OPTION;
const int U_BRK_VARIABLE_REDFINITION;
const int U_BUFFER_OVERFLOW_ERROR;
const int U_CE_NOT_FOUND_ERROR;
const int U_COLLATOR_VERSION_MISMATCH;
const int U_DIFFERENT_UCA_VERSION;
const int U_ENUM_OUT_OF_SYNC_ERROR;
const int U_ERROR_LIMIT;
const int U_ERROR_WARNING_LIMIT;
const int U_ERROR_WARNING_START;
const int U_FILE_ACCESS_ERROR;
const int U_FMT_PARSE_ERROR_LIMIT;
const int U_FMT_PARSE_ERROR_START;
const int U_IDNA_ACE_PREFIX_ERROR;
const int U_IDNA_CHECK_BIDI_ERROR;
const int U_IDNA_DOMAIN_NAME_TOO_LONG_ERROR;
const int U_IDNA_ERROR_LIMIT;
const int U_IDNA_ERROR_START;
const int U_IDNA_LABEL_TOO_LONG_ERROR;
const int U_IDNA_PROHIBITED_ERROR;
const int U_IDNA_STD3_ASCII_RULES_ERROR;
const int U_IDNA_UNASSIGNED_ERROR;
const int U_IDNA_VERIFICATION_ERROR;
const int U_IDNA_ZERO_LENGTH_LABEL_ERROR;
const int U_ILLEGAL_ARGUMENT_ERROR;
const int U_ILLEGAL_CHARACTER;
const int U_ILLEGAL_CHAR_FOUND;
const int U_ILLEGAL_CHAR_IN_SEGMENT;
const int U_ILLEGAL_ESCAPE_SEQUENCE;
const int U_ILLEGAL_PAD_POSITION;
const int U_INDEX_OUTOFBOUNDS_ERROR;
const int U_INTERNAL_PROGRAM_ERROR;
const int U_INTERNAL_TRANSLITERATOR_ERROR;
const int U_INVALID_CHAR_FOUND;
const int U_INVALID_FORMAT_ERROR;
const int U_INVALID_FUNCTION;
const int U_INVALID_ID;
const int U_INVALID_PROPERTY_PATTERN;
const int U_INVALID_RBT_SYNTAX;
const int U_INVALID_STATE_ERROR;
const int U_INVALID_TABLE_FILE;
const int U_INVALID_TABLE_FORMAT;
const int U_INVARIANT_CONVERSION_ERROR;
const int U_MALFORMED_EXPONENTIAL_PATTERN;
const int U_MALFORMED_PRAGMA;
const int U_MALFORMED_RULE;
const int U_MALFORMED_SET;
const int U_MALFORMED_SYMBOL_REFERENCE;
const int U_MALFORMED_UNICODE_ESCAPE;
const int U_MALFORMED_VARIABLE_DEFINITION;
const int U_MALFORMED_VARIABLE_REFERENCE;
const int U_MEMORY_ALLOCATION_ERROR;
const int U_MESSAGE_PARSE_ERROR;
const int U_MISMATCHED_SEGMENT_DELIMITERS;
const int U_MISPLACED_ANCHOR_START;
const int U_MISPLACED_COMPOUND_FILTER;
const int U_MISPLACED_CURSOR_OFFSET;
const int U_MISPLACED_QUANTIFIER;
const int U_MISSING_OPERATOR;
const int U_MISSING_RESOURCE_ERROR;
const int U_MISSING_SEGMENT_CLOSE;
const int U_MULTIPLE_ANTE_CONTEXTS;
const int U_MULTIPLE_COMPOUND_FILTERS;
const int U_MULTIPLE_CURSORS;
const int U_MULTIPLE_DECIMAL_SEPARATORS;
const int U_MULTIPLE_EXPONENTIAL_SYMBOLS;
const int U_MULTIPLE_PAD_SPECIFIERS;
const int U_MULTIPLE_PERCENT_SYMBOLS;
const int U_MULTIPLE_PERMILL_SYMBOLS;
const int U_MULTIPLE_POST_CONTEXTS;
const int U_NO_SPACE_AVAILABLE;
const int U_NO_WRITE_PERMISSION;
const int U_PARSE_ERROR;
const int U_PARSE_ERROR_LIMIT;
const int U_PARSE_ERROR_START;
const int U_PATTERN_SYNTAX_ERROR;
const int U_PRIMARY_TOO_LONG_ERROR;
const int U_REGEX_BAD_ESCAPE_SEQUENCE;
const int U_REGEX_BAD_INTERVAL;
const int U_REGEX_ERROR_LIMIT;
const int U_REGEX_ERROR_START;
const int U_REGEX_INTERNAL_ERROR;
const int U_REGEX_INVALID_BACK_REF;
const int U_REGEX_INVALID_FLAG;
const int U_REGEX_INVALID_STATE;
const int U_REGEX_LOOK_BEHIND_LIMIT;
const int U_REGEX_MAX_LT_MIN;
const int U_REGEX_MISMATCHED_PAREN;
const int U_REGEX_NUMBER_TOO_BIG;
const int U_REGEX_PROPERTY_SYNTAX;
const int U_REGEX_RULE_SYNTAX;
const int U_REGEX_SET_CONTAINS_STRING;
const int U_REGEX_UNIMPLEMENTED;
const int U_RESOURCE_TYPE_MISMATCH;
const int U_RULE_MASK_ERROR;
const int U_SAFECLONE_ALLOCATED_WARNING;
const int U_SORT_KEY_TOO_SHORT_WARNING;
const int U_STANDARD_ERROR_LIMIT;
const int U_STATE_OLD_WARNING;
const int U_STATE_TOO_OLD_ERROR;
const int U_STRINGPREP_CHECK_BIDI_ERROR;
const int U_STRINGPREP_PROHIBITED_ERROR;
const int U_STRINGPREP_UNASSIGNED_ERROR;
const int U_STRING_NOT_TERMINATED_WARNING;
const int U_TOO_MANY_ALIASES_ERROR;
const int U_TRAILING_BACKSLASH;
const int U_TRUNCATED_CHAR_FOUND;
const int U_UNCLOSED_SEGMENT;
const int U_UNDEFINED_SEGMENT_REFERENCE;
const int U_UNDEFINED_VARIABLE;
const int U_UNEXPECTED_TOKEN;
const int U_UNMATCHED_BRACES;
const int U_UNQUOTED_SPECIAL;
const int U_UNSUPPORTED_ATTRIBUTE;
const int U_UNSUPPORTED_ERROR;
const int U_UNSUPPORTED_ESCAPE_SEQUENCE;
const int U_UNSUPPORTED_PROPERTY;
const int U_UNTERMINATED_QUOTE;
const int U_USELESS_COLLATOR_ERROR;
const int U_USING_DEFAULT_WARNING;
const int U_USING_FALLBACK_WARNING;
const int U_VARIABLE_RANGE_EXHAUSTED;
const int U_VARIABLE_RANGE_OVERLAP;
const int U_ZERO_ERROR;

const int GRAPHEME_EXTR_COUNT;
const int GRAPHEME_EXTR_MAXBYTES;
const int GRAPHEME_EXTR_MAXCHARS;

<<__PHPStdLib>>
function intl_get_error_code(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intl_get_error_message(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intl_error_name(int $error_code): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intl_is_failure(int $error_code): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_asort(
  $obj,
  inout $arr,
  int $sort_flag = Collator::SORT_REGULAR,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_compare($obj, $str1, $str2): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_create(string $locale): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_get_attribute($obj, int $attr): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_get_error_code($obj): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_get_error_message($obj): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_get_locale($obj, int $type = 0): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_get_sort_key($obj, string $str): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_get_strength($obj): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_set_attribute(
  $obj,
  int $attr,
  int $val,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_set_strength(
  $obj,
  int $strength,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_sort_with_sort_keys(
  $obj,
  inout $arr,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function collator_sort(
  $obj,
  inout $arr,
  int $sort_flag = Collator::SORT_REGULAR,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function idn_to_ascii(
  string $domain,
  int $options = 0,
  int $variant = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function idn_to_unicode(
  string $domain,
  int $options = 0,
  int $variant = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function idn_to_utf8(
  string $domain,
  int $options = 0,
  int $variant = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_create(
  $locale,
  $date_type,
  $time_type,
  $timezone_str = null,
  $calendar = null,
  $pattern = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_format(
  $args = null,
  $array = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_format_object(
  $object,
  $format = null,
  $locale = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_calendar($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_calendar_object($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_datetype($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_error_code($nf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_error_message($coll): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_locale($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_pattern($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_timetype($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_timezone($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_get_timezone_id($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_is_lenient($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_localtime(
  $formatter,
  $string,
  inout $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_parse(
  $formatter,
  $string,
  inout $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_set_calendar($mf, $calendar): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_set_lenient($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_set_pattern($mf, $pattern): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_set_timezone($mf, $timezone): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_set_timezone_id($mf, $timezone): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_extract(
  string $haystack,
  int $size,
  int $extract_type,
  int $start,
  inout ?int $next,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_stripos(
  string $haystack,
  string $needle,
  int $offset = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_stristr(
  string $haystack,
  string $needle,
  bool $before_needle = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_strlen(string $string): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_strpos(
  string $haystack,
  string $needle,
  int $offset = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_strripos(
  string $haystack,
  string $needle,
  int $offset = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_strrpos(
  string $haystack,
  string $needle,
  int $offset = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_strstr(
  string $haystack,
  string $needle,
  bool $before_needle = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function grapheme_substr(
  string $string,
  int $start,
  $length = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_add(
  IntlCalendar $calendar,
  int $field,
  int $amount,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_after(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_before(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_clear(
  IntlCalendar $calendar,
  $field = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_create_instance(
  $timeZone = null,
  string $locale = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_equals(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_field_difference(
  IntlCalendar $calendar,
  $when,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_from_date_time($dateTime): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_actual_maximum(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_actual_minimum(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_available_locales(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_day_of_week_type(
  IntlCalendar $calendar,
  int $dayOfWeek,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_error_code(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_error_message(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_first_day_of_week(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_greatest_minimum(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_keyword_values_for_locale(
  string $key,
  string $locale,
  bool $commonlyUsed,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_least_maximum(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_locale(
  IntlCalendar $calendar,
  int $localeType,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_maximum(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_minimal_days_in_first_week(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_minimum(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_now(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_repeated_wall_time_option(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_skipped_wall_time_option(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_time(IntlCalendar $calendar): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_time_zone(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_type(IntlCalendar $calendar): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_weekend_transition(
  IntlCalendar $calendar,
  int $dayOfWeek,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_in_daylight_time(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_is_equivalent_to(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_is_lenient(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_is_set(
  IntlCalendar $calendar,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_is_weekend(
  IntlCalendar $calendar,
  $date = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_roll(
  IntlCalendar $calendar,
  int $field,
  $amountOrUpOrDown = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set(
  IntlCalendar $calendar,
  int $fieldOrYear,
  int $valueOrMonth,
  $dayOfMonth = null,
  $hour = null,
  $minute = null,
  $second = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set_first_day_of_week(
  IntlCalendar $calendar,
  int $dayOfWeek,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set_lenient(
  IntlCalendar $calendar,
  bool $isLenient,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set_minimal_days_in_first_week(
  IntlCalendar $calendar,
  int $numberOfDays,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set_repeated_wall_time_option(
  IntlCalendar $calendar,
  int $wallTimeOption,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set_skipped_wall_time_option(
  IntlCalendar $calendar,
  int $wallTimeOption,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set_time(
  IntlCalendar $calendar,
  $date,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_set_time_zone(
  IntlCalendar $calendar,
  $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_to_date_time(
  IntlCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlgregcal_create_instance(
  $timeZoneOrYear = null,
  $localeOrMonth = null,
  $dayOfMonth = null,
  $hour = null,
  $minute = null,
  $second = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlgregcal_get_gregorian_change(
  IntlGregorianCalendar $calendar,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlgregcal_is_leap_year(
  IntlGregorianCalendar $calendar,
  int $year,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlgregcal_set_gregorian_change(
  IntlGregorianCalendar $calendar,
  float $date,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_count_equivalent_ids(
  string $zoneId,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_create_default(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_create_enumeration(
  $countryOrRawOffset = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_create_time_zone(string $zoneId): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_create_time_zone_id_enumeration(
  int $zoneType,
  string $region = "",
  $rawOffset = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_from_date_time_zone(
  DateTimeZone $dateTimeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_canonical_id(
  string $zoneId,
  inout $isSystemID,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_display_name(
  IntlTimeZone $timeZone,
  bool $isDaylight = false,
  int $style = IntlTimeZone::DISPLAY_LONG,
  string $locale = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_dst_savings(
  IntlTimeZone $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_equivalent_id(
  string $zoneId,
  int $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_error_code(
  IntlTimeZone $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_error_message(
  IntlTimeZone $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_gmt(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_id(IntlTimeZone $timeZone): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_offset(
  IntlTimeZone $timeZone,
  float $date,
  bool $local,
  inout int $rawOffset,
  inout int $dstOffset,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_raw_offset(
  IntlTimeZone $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_region(string $zoneId): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_tz_data_version(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_unknown(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_has_same_rules(
  IntlTimeZone $timeZone,
  ?IntlTimeZone $otherTimeZone = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_to_date_time_zone(
  IntlTimeZone $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_use_daylight_time(
  IntlTimeZone $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_accept_from_http(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_canonicalize(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_compose($arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_filter_matches(
  $arg1,
  $arg2,
  $arg3,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_all_variants(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_default(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_display_language(
  string $arg1,
  string $arg2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_display_name(
  string $arg1,
  string $arg2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_display_region(
  string $arg1,
  string $arg2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_display_script(
  string $arg1,
  string $arg2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_display_variant(
  string $arg1,
  string $arg2,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_keywords(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_primary_language(
  string $arg1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_region(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_script(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_lookup(
  $arg1,
  string $arg2,
  bool $arg3,
  string $arg4,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_parse(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_set_default(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_create(
  string $locale,
  string $pattern,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_format($nf, $args): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_format_message(
  string $locale,
  string $pattern,
  $args,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_get_error_code($nf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_get_error_message($coll): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_get_locale($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_get_pattern($mf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_parse($nf, string $source): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_parse_message(
  string $locale,
  string $pattern,
  string $source,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_set_pattern($mf, string $pattern): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function normalizer_is_normalized(
  string $input,
  int $form = Normalizer::FORM_C,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function normalizer_normalize(
  string $input,
  int $form = Normalizer::FORM_C,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_create(
  $locale,
  $style,
  $pattern = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_format($nf, $num, $type = null): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_format_currency(
  $nf,
  $num,
  $currency,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_attribute($nf, $attr): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_error_code($nf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_error_message($nf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_locale($nf, $type = null): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_pattern($nf): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_symbol($nf, $attr): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_text_attribute($nf, $attr): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_parse(
  $formatter,
  $string,
  $type,
  inout $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_parse_currency(
  $formatter,
  $string,
  inout $currency,
  inout $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_set_attribute($nf, $attr, $value): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_set_pattern($nf, $pattern): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_set_symbol($nf, $attr, $symbol): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_set_text_attribute(
  $nf,
  $attr,
  $value,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_count($bundle): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_create(
  $locale,
  $bundlename,
  bool $fallback = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_get($bundle, $index): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_get_error_code($bundle): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_get_error_message(
  $bundle,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_locales(
  string $bundlename,
): HH\FIXME\MISSING_RETURN_TYPE;

class Collator {
  const int SORT_REGULAR = 0;
  const int SORT_NUMERIC = 0;
  const int SORT_STRING = 0;
  const int FRENCH_COLLATION = 0;
  const int ALTERNATE_HANDLING = 0;
  const int CASE_FIRST = 0;
  const int CASE_LEVEL = 0;
  const int NORMALIZATION_MODE = 0;
  const int STRENGTH = 0;
  const int HIRAGANA_QUATERNARY_MODE = 0;
  const int NUMERIC_COLLATION = 0;
  const int DEFAULT_VALUE = 0;
  const int PRIMARY = 0;
  const int SECONDARY = 0;
  const int TERTIARY = 0;
  const int DEFAULT_STRENGTH = 0;
  const int QUATERNARY = 0;
  const int IDENTICAL = 0;
  const int OFF = 0;
  const int ON = 0;
  const int SHIFTED = 0;
  const int NON_IGNORABLE = 0;
  const int LOWER_FIRST = 0;
  const int UPPER_FIRST = 0;

  public function __construct(string $locale);
  public function asort(
    inout $arr,
    int $sort_flag = Collator::SORT_REGULAR,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function compare($str1, $str2): HH\FIXME\MISSING_RETURN_TYPE;
  static public function create(string $locale): HH\FIXME\MISSING_RETURN_TYPE;
  public function getAttribute(int $attr): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getLocale(int $type = 0): HH\FIXME\MISSING_RETURN_TYPE;
  public function getSortKey(string $str): HH\FIXME\MISSING_RETURN_TYPE;
  public function getStrength(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttribute(
    int $attr,
    int $val,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setStrength(int $strength): HH\FIXME\MISSING_RETURN_TYPE;
  public function sortWithSortKeys(inout $arr): HH\FIXME\MISSING_RETURN_TYPE;
  public function sort(
    inout $arr,
    int $sort_flag = Collator::SORT_REGULAR,
  ): HH\FIXME\MISSING_RETURN_TYPE;
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

  public static function acceptFromHttp(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function canonicalize(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function composeLocale($arg1): HH\FIXME\MISSING_RETURN_TYPE;
  public static function filterMatches(
    $arg1,
    $arg2,
    $arg3,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getAllVariants(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getDefault(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getDisplayLanguage(
    string $arg1,
    string $arg2,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getDisplayName(
    string $arg1,
    string $arg2,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getDisplayRegion(
    string $arg1,
    string $arg2,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getDisplayScript(
    string $arg1,
    string $arg2,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getDisplayVariant(
    string $arg1,
    string $arg2,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getKeywords(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getPrimaryLanguage(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getRegion(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getScript(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
  public static function lookup(
    $arg1,
    string $arg2,
    bool $arg3,
    string $arg4,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function parseLocale(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function setDefault(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
}

class Normalizer {
  const int NONE = 0;
  const int FORM_D = 0;
  const int NFD = 0;
  const int FORM_KD = 0;
  const int NFKD = 0;
  const int FORM_C = 0;
  const int NFC = 0;
  const int FORM_KC = 0;
  const int NFKC = 0;

  static public function isNormalized(
    string $input,
    int $form = Normalizer::FORM_C,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  static public function normalize(
    string $input,
    int $form = Normalizer::FORM_C,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}

class MessageFormatter {
  public function __construct(string $locale, string $pattern);
  public static function create(
    string $locale,
    string $pattern,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function format($args): HH\FIXME\MISSING_RETURN_TYPE;
  public static function formatMessage(
    string $locale,
    string $pattern,
    $args,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getLocale(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getPattern(): HH\FIXME\MISSING_RETURN_TYPE;
  public function parse(string $source): HH\FIXME\MISSING_RETURN_TYPE;
  public static function parseMessage(
    string $locale,
    string $pattern,
    string $args,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setPattern(string $pattern): HH\FIXME\MISSING_RETURN_TYPE;
}

class IntlDateFormatter {
  const int FULL = 0;
  const int LONG = 1;
  const int MEDIUM = 2;
  const int SHORT = 3;
  const int NONE = -1;
  const int GREGORIAN = 1;
  const int TRADITIONAL = 0;

  public function __construct(
    string $locale,
    int $datetype,
    int $timetype,
    $timezone = null,
    $calendar = null,
    string $pattern = "",
  );
  public static function create(
    $locale,
    $datetype,
    $timetype,
    $timezone = null,
    $calendar = null,
    $pattern = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function format($args = null): HH\FIXME\MISSING_RETURN_TYPE;
  public static function FormatObject(
    $object,
    $format = null,
    $locale = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getCalendar(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getCalendarObject(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDateType(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getLocale(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getPattern(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getTimeType(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getTimezone(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getTimezoneId(): HH\FIXME\MISSING_RETURN_TYPE;
  public function isLenient(): HH\FIXME\MISSING_RETURN_TYPE;
  public function localtime(
    string $string,
    inout $position,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function parse(string $string): HH\FIXME\MISSING_RETURN_TYPE;
  public function parseWithPosition(
    string $string,
    inout $position,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setCalendar($which): HH\FIXME\MISSING_RETURN_TYPE;
  public function setLenient(bool $lenient): HH\FIXME\MISSING_RETURN_TYPE;
  public function setPattern(string $pattern): HH\FIXME\MISSING_RETURN_TYPE;
  public function setTimezone($zone): HH\FIXME\MISSING_RETURN_TYPE;
  public function setTimezoneId(string $zone): HH\FIXME\MISSING_RETURN_TYPE;
}

class ResourceBundle<T> implements Traversable<T> {
  public function __construct($locale, $bundlename, bool $fallback = true);
  public function count(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function create(
    $locale,
    $bundlename,
    bool $fallback = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function get(
    $index,
    bool $fallback = true,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getLocales(
    string $bundlename,
  ): HH\FIXME\MISSING_RETURN_TYPE;
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

  private function __construct();
  public static function countEquivalentIDs(
    string $zoneId,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createDefault(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createEnumeration(
    $countryOrRawOffset = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createTimeZone(
    string $zoneId,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createTimeZoneIDEnumeration(
    int $zoneType,
    string $region = "",
    $rawOffset = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function fromDateTimeZone(
    $zoneId,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getCanonicalID(
    string $zoneId,
    inout $isSystemID,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDSTSavings(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDisplayName(
    bool $isDaylight = false,
    int $style = IntlTimeZone::DISPLAY_LONG,
    string $locale = "",
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getEquivalentID(
    string $zoneId,
    int $index,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getGMT(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getID(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getOffset(
    float $date,
    bool $local,
    inout int $rawOffset,
    inout int $dstOffset,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getRawOffset(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getRegion(
    string $zoneId,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getTZDataVersion(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getUnknown(): HH\FIXME\MISSING_RETURN_TYPE;
  public function hasSameRules(
    IntlTimeZone $otherTimeZone,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function toDateTimeZone(): HH\FIXME\MISSING_RETURN_TYPE;
  public function useDaylightTime(): HH\FIXME\MISSING_RETURN_TYPE;
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

  private function __construct();
  public function add(int $field, int $amount): HH\FIXME\MISSING_RETURN_TYPE;
  public function after(IntlCalendar $calendar): HH\FIXME\MISSING_RETURN_TYPE;
  public function before(IntlCalendar $calendar): HH\FIXME\MISSING_RETURN_TYPE;
  public function clear($field = null): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createInstance(
    $timeZone = null,
    string $locale = "",
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function equals(IntlCalendar $calendar): HH\FIXME\MISSING_RETURN_TYPE;
  public function fieldDifference(
    $when,
    int $field,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function fromDateTime($dateTime): HH\FIXME\MISSING_RETURN_TYPE;
  public function get(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public function getActualMaximum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public function getActualMinimum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getAvailableLocales(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDayOfWeekType(
    int $dayOfWeek,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getFirstDayOfWeek(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getGreatestMinimum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getKeywordValuesForLocale(
    string $key,
    string $locale,
    bool $commonlyUsed,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getLeastMaximum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public function getLocale(int $localeType): HH\FIXME\MISSING_RETURN_TYPE;
  public function getMaximum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public function getMinimalDaysInFirstWeek(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getMinimum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getNow(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getRepeatedWallTimeOption(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getSkippedWallTimeOption(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getTime(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getTimezone(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getType(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getWeekendTransition(
    int $dayOfWeek,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function inDaylightTime(): HH\FIXME\MISSING_RETURN_TYPE;
  public function isEquivalentTo(
    IntlCalendar $calendar,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function isLenient(): HH\FIXME\MISSING_RETURN_TYPE;
  public function isSet($field): HH\FIXME\MISSING_RETURN_TYPE;
  public function isWeekend($date = null): HH\FIXME\MISSING_RETURN_TYPE;
  public function roll(
    int $field,
    $amountOrUpOrDown,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function set(
    int $fieldOrYear,
    int $valueOrMonth,
    $dayOfMonth = null,
    $hour = null,
    $minute = null,
    $second = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setFirstDayOfWeek(
    int $dayOfWeek,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setLenient(bool $isLenient): HH\FIXME\MISSING_RETURN_TYPE;
  public function setMinimalDaysInFirstWeek(
    int $numberOfDays,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setRepeatedWallTimeOption(
    int $wallTimeOption,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setSkippedWallTimeOption(
    int $wallTimeOption,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setTime($date): HH\FIXME\MISSING_RETURN_TYPE;
  public function setTimezone($timeZone): HH\FIXME\MISSING_RETURN_TYPE;
  public function toDateTime(): HH\FIXME\MISSING_RETURN_TYPE;
}

class IntlGregorianCalendar extends IntlCalendar {
  public function getGregorianChange(): HH\FIXME\MISSING_RETURN_TYPE;
  public function isLeapYear(int $year): HH\FIXME\MISSING_RETURN_TYPE;
  public function setGregorianChange(float $date): HH\FIXME\MISSING_RETURN_TYPE;
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
  const int DONE = -1;
  const int WORD_NONE = 0;
  const int WORD_NONE_LIMIT = 100;
  const int WORD_NUMBER = 100;
  const int WORD_LETTER = 200;
  const int WORD_KANA = 300;
  const int WORD_KANA_LIMIT = 400;
  const int WORD_IDEO = 400;
  const int WORD_IDEO_LIMIT = 500;
  const int LINE_SOFT = 0;
  const int LINE_SOFT_LIMIT = 100;
  const int LINE_HARD = 100;
  const int LINE_HARD_LIMIT = 200;
  const int SENTENCE_TERM = 0;
  const int SENTENCE_SEP = 100;
  const int WORD_NUMBER_LIMIT = 200;
  const int WORD_LETTER_LIMIT = 300;
  const int SENTENCE_TERM_LIMIT = 100;
  const int SENTENCE_SEP_LIMIT = 200;

  // Methods
  public static function createCharacterInstance(
    $locale = null,
  ): IntlBreakIterator;
  public static function createCodePointInstance(): IntlCodePointBreakIterator;
  public static function createLineInstance($locale = null): IntlBreakIterator;
  public static function createSentenceInstance(
    $locale = null,
  ): IntlBreakIterator;
  public static function createTitleInstance($locale = null): IntlBreakIterator;
  public static function createWordInstance($locale = null): IntlBreakIterator;
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
  public function getBreakIterator(): IntlBreakIterator {}
}
