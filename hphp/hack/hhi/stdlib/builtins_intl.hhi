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
function intl_get_error_code(): int;
<<__PHPStdLib>>
function intl_get_error_message(): string;
<<__PHPStdLib>>
function intl_error_name(int $error_code): string;
<<__PHPStdLib>>
function intl_is_failure(int $error_code): bool;
<<__PHPStdLib>>
function collator_asort(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  inout HH\FIXME\MISSING_PARAM_TYPE $arr,
  int $sort_flag = Collator::SORT_REGULAR,
): bool;
<<__PHPStdLib>>
function collator_compare(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  HH\FIXME\MISSING_PARAM_TYPE $str1,
  HH\FIXME\MISSING_PARAM_TYPE $str2,
): int;
<<__PHPStdLib>>
function collator_create(string $locale): Collator;
<<__PHPStdLib>>
function collator_get_attribute(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  int $attr,
): int;
<<__PHPStdLib>>
function collator_get_error_code(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
): int;
<<__PHPStdLib>>
function collator_get_error_message(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
): string;
<<__PHPStdLib>>
function collator_get_locale(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  int $type = 0,
): string;
<<__PHPStdLib>>
function collator_get_sort_key(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  string $str,
): string;
<<__PHPStdLib>>
function collator_get_strength(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
): int;
<<__PHPStdLib>>
function collator_set_attribute(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  int $attr,
  int $val,
): bool;
<<__PHPStdLib>>
function collator_set_strength(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  int $strength,
): bool;
<<__PHPStdLib>>
function collator_sort_with_sort_keys(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  inout HH\FIXME\MISSING_PARAM_TYPE $arr,
): bool;
<<__PHPStdLib>>
function collator_sort(
  HH\FIXME\MISSING_PARAM_TYPE $obj,
  inout HH\FIXME\MISSING_PARAM_TYPE $arr,
  int $sort_flag = Collator::SORT_REGULAR,
): bool;
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
  HH\FIXME\MISSING_PARAM_TYPE $locale,
  HH\FIXME\MISSING_PARAM_TYPE $date_type,
  HH\FIXME\MISSING_PARAM_TYPE $time_type,
  HH\FIXME\MISSING_PARAM_TYPE $timezone_str = null,
  HH\FIXME\MISSING_PARAM_TYPE $calendar = null,
  HH\FIXME\MISSING_PARAM_TYPE $pattern = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_format(
  HH\FIXME\MISSING_PARAM_TYPE $args = null,
  HH\FIXME\MISSING_PARAM_TYPE $array = null,
): string;
<<__PHPStdLib>>
function datefmt_format_object(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  HH\FIXME\MISSING_PARAM_TYPE $format = null,
  HH\FIXME\MISSING_PARAM_TYPE $locale = null,
): string;
<<__PHPStdLib>>
function datefmt_get_calendar(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): int;
<<__PHPStdLib>>
function datefmt_get_calendar_object(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): IntlCalendar;
<<__PHPStdLib>>
function datefmt_get_datetype(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): int;
<<__PHPStdLib>>
function datefmt_get_error_code(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
): int;
<<__PHPStdLib>>
function datefmt_get_error_message(
  HH\FIXME\MISSING_PARAM_TYPE $coll,
): string;
<<__PHPStdLib>>
function datefmt_get_locale(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): string;
<<__PHPStdLib>>
function datefmt_get_pattern(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): string;
<<__PHPStdLib>>
function datefmt_get_timetype(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): int;
<<__PHPStdLib>>
function datefmt_get_timezone(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): IntlTimeZone;
<<__PHPStdLib>>
function datefmt_get_timezone_id(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): string;
<<__PHPStdLib>>
function datefmt_is_lenient(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): bool;
<<__PHPStdLib>>
function datefmt_localtime(
  HH\FIXME\MISSING_PARAM_TYPE $formatter,
  HH\FIXME\MISSING_PARAM_TYPE $string,
  inout HH\FIXME\MISSING_PARAM_TYPE $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_parse(
  HH\FIXME\MISSING_PARAM_TYPE $formatter,
  HH\FIXME\MISSING_PARAM_TYPE $string,
  inout HH\FIXME\MISSING_PARAM_TYPE $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function datefmt_set_calendar(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
  HH\FIXME\MISSING_PARAM_TYPE $calendar,
): bool;
<<__PHPStdLib>>
function datefmt_set_lenient(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): bool;
<<__PHPStdLib>>
function datefmt_set_pattern(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
): bool;
<<__PHPStdLib>>
function datefmt_set_timezone(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
  HH\FIXME\MISSING_PARAM_TYPE $timezone,
): bool;
<<__PHPStdLib>>
function datefmt_set_timezone_id(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
  HH\FIXME\MISSING_PARAM_TYPE $timezone,
): bool;
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
  HH\FIXME\MISSING_PARAM_TYPE $length = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_add(
  IntlCalendar $calendar,
  int $field,
  int $amount,
): bool;
<<__PHPStdLib>>
function intlcal_after(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): bool;
<<__PHPStdLib>>
function intlcal_before(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): bool;
<<__PHPStdLib>>
function intlcal_clear(
  IntlCalendar $calendar,
  HH\FIXME\MISSING_PARAM_TYPE $field = null,
): bool;
<<__PHPStdLib>>
function intlcal_create_instance(
  HH\FIXME\MISSING_PARAM_TYPE $timeZone = null,
  string $locale = "",
): IntlCalendar;
<<__PHPStdLib>>
function intlcal_equals(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): bool;
<<__PHPStdLib>>
function intlcal_field_difference(
  IntlCalendar $calendar,
  HH\FIXME\MISSING_PARAM_TYPE $when,
  int $field,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_from_date_time(
  HH\FIXME\MISSING_PARAM_TYPE $dateTime,
): IntlCalendar;
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
): int;
<<__PHPStdLib>>
function intlcal_get_error_message(
  IntlCalendar $calendar,
): string;
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
function intlcal_get_now(): float;
<<__PHPStdLib>>
function intlcal_get_repeated_wall_time_option(
  IntlCalendar $calendar,
): int;
<<__PHPStdLib>>
function intlcal_get_skipped_wall_time_option(
  IntlCalendar $calendar,
): int;
<<__PHPStdLib>>
function intlcal_get_time(IntlCalendar $calendar): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intlcal_get_time_zone(
  IntlCalendar $calendar,
): IntlTimeZone;
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
): bool;
<<__PHPStdLib>>
function intlcal_is_equivalent_to(
  IntlCalendar $calendar,
  IntlCalendar $otherCalendar,
): bool;
<<__PHPStdLib>>
function intlcal_is_lenient(
  IntlCalendar $calendar,
): bool;
<<__PHPStdLib>>
function intlcal_is_set(
  IntlCalendar $calendar,
  int $field,
): bool;
<<__PHPStdLib>>
function intlcal_is_weekend(
  IntlCalendar $calendar,
  HH\FIXME\MISSING_PARAM_TYPE $date = null,
): bool;
<<__PHPStdLib>>
function intlcal_roll(
  IntlCalendar $calendar,
  int $field,
  HH\FIXME\MISSING_PARAM_TYPE $amountOrUpOrDown = null,
): bool;
<<__PHPStdLib>>
function intlcal_set(
  IntlCalendar $calendar,
  int $fieldOrYear,
  int $valueOrMonth,
  HH\FIXME\MISSING_PARAM_TYPE $dayOfMonth = null,
  HH\FIXME\MISSING_PARAM_TYPE $hour = null,
  HH\FIXME\MISSING_PARAM_TYPE $minute = null,
  HH\FIXME\MISSING_PARAM_TYPE $second = null,
): bool;
<<__PHPStdLib>>
function intlcal_set_first_day_of_week(
  IntlCalendar $calendar,
  int $dayOfWeek,
): bool;
<<__PHPStdLib>>
function intlcal_set_lenient(
  IntlCalendar $calendar,
  bool $isLenient,
): bool;
<<__PHPStdLib>>
function intlcal_set_minimal_days_in_first_week(
  IntlCalendar $calendar,
  int $numberOfDays,
): bool;
<<__PHPStdLib>>
function intlcal_set_repeated_wall_time_option(
  IntlCalendar $calendar,
  int $wallTimeOption,
): bool;
<<__PHPStdLib>>
function intlcal_set_skipped_wall_time_option(
  IntlCalendar $calendar,
  int $wallTimeOption,
): bool;
<<__PHPStdLib>>
function intlcal_set_time(
  IntlCalendar $calendar,
  HH\FIXME\MISSING_PARAM_TYPE $date,
): bool;
<<__PHPStdLib>>
function intlcal_set_time_zone(
  IntlCalendar $calendar,
  HH\FIXME\MISSING_PARAM_TYPE $timeZone,
): bool;
<<__PHPStdLib>>
function intlcal_to_date_time(
  IntlCalendar $calendar,
): DateTime;
<<__PHPStdLib>>
function intlgregcal_create_instance(
  HH\FIXME\MISSING_PARAM_TYPE $timeZoneOrYear = null,
  HH\FIXME\MISSING_PARAM_TYPE $localeOrMonth = null,
  HH\FIXME\MISSING_PARAM_TYPE $dayOfMonth = null,
  HH\FIXME\MISSING_PARAM_TYPE $hour = null,
  HH\FIXME\MISSING_PARAM_TYPE $minute = null,
  HH\FIXME\MISSING_PARAM_TYPE $second = null,
): IntlGregorianCalendar;
<<__PHPStdLib>>
function intlgregcal_get_gregorian_change(
  IntlGregorianCalendar $calendar,
): float;
<<__PHPStdLib>>
function intlgregcal_is_leap_year(
  IntlGregorianCalendar $calendar,
  int $year,
): bool;
<<__PHPStdLib>>
function intlgregcal_set_gregorian_change(
  IntlGregorianCalendar $calendar,
  float $date,
): bool;
<<__PHPStdLib>>
function intltz_count_equivalent_ids(
  string $zoneId,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_create_default(): IntlTimeZone;
<<__PHPStdLib>>
function intltz_create_enumeration(
  HH\FIXME\MISSING_PARAM_TYPE $countryOrRawOffset = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_create_time_zone(string $zoneId): IntlTimeZone;
<<__PHPStdLib>>
function intltz_create_time_zone_id_enumeration(
  int $zoneType,
  string $region = "",
  HH\FIXME\MISSING_PARAM_TYPE $rawOffset = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_from_date_time_zone(
  DateTimeZone $dateTimeZone,
): IntlTimeZone;
<<__PHPStdLib>>
function intltz_get_canonical_id(
  string $zoneId,
  inout HH\FIXME\MISSING_PARAM_TYPE $isSystemID,
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
): int;
<<__PHPStdLib>>
function intltz_get_equivalent_id(
  string $zoneId,
  int $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_error_code(
  IntlTimeZone $timeZone,
): int;
<<__PHPStdLib>>
function intltz_get_error_message(
  IntlTimeZone $timeZone,
): string;
<<__PHPStdLib>>
function intltz_get_gmt(): IntlTimeZone;
<<__PHPStdLib>>
function intltz_get_id(IntlTimeZone $timeZone): string;
<<__PHPStdLib>>
function intltz_get_offset(
  IntlTimeZone $timeZone,
  float $date,
  bool $local,
  inout int $rawOffset,
  inout int $dstOffset,
): bool;
<<__PHPStdLib>>
function intltz_get_raw_offset(
  IntlTimeZone $timeZone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_region(string $zoneId): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_tz_data_version(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intltz_get_unknown(): IntlTimeZone;
<<__PHPStdLib>>
function intltz_has_same_rules(
  IntlTimeZone $timeZone,
  ?IntlTimeZone $otherTimeZone = null,
): bool;
<<__PHPStdLib>>
function intltz_to_date_time_zone(
  IntlTimeZone $timeZone,
): DateTimeZone;
<<__PHPStdLib>>
function intltz_use_daylight_time(
  IntlTimeZone $timeZone,
): bool;
<<__PHPStdLib>>
function locale_accept_from_http(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_canonicalize(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_compose(
  HH\FIXME\MISSING_PARAM_TYPE $arg1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_filter_matches(
  HH\FIXME\MISSING_PARAM_TYPE $arg1,
  HH\FIXME\MISSING_PARAM_TYPE $arg2,
  HH\FIXME\MISSING_PARAM_TYPE $arg3,
): bool;
<<__PHPStdLib>>
function locale_get_all_variants(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_default(): string;
<<__PHPStdLib>>
function locale_get_display_language(
  string $arg1,
  string $arg2,
): string;
<<__PHPStdLib>>
function locale_get_display_name(
  string $arg1,
  string $arg2,
): string;
<<__PHPStdLib>>
function locale_get_display_region(
  string $arg1,
  string $arg2,
): string;
<<__PHPStdLib>>
function locale_get_display_script(
  string $arg1,
  string $arg2,
): string;
<<__PHPStdLib>>
function locale_get_display_variant(
  string $arg1,
  string $arg2,
): string;
<<__PHPStdLib>>
function locale_get_keywords(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_primary_language(
  string $arg1,
): string;
<<__PHPStdLib>>
function locale_get_region(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_get_script(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_lookup(
  HH\FIXME\MISSING_PARAM_TYPE $arg1,
  string $arg2,
  bool $arg3,
  string $arg4,
): string;
<<__PHPStdLib>>
function locale_parse(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function locale_set_default(string $arg1): bool;
<<__PHPStdLib>>
function msgfmt_create(
  string $locale,
  string $pattern,
): ?MessageFormatter;
<<__PHPStdLib>>
function msgfmt_format(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $args,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_format_message(
  string $locale,
  string $pattern,
  HH\FIXME\MISSING_PARAM_TYPE $args,
): string;
<<__PHPStdLib>>
function msgfmt_get_error_code(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
): int;
<<__PHPStdLib>>
function msgfmt_get_error_message(
  HH\FIXME\MISSING_PARAM_TYPE $coll,
): string;
<<__PHPStdLib>>
function msgfmt_get_locale(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): string;
<<__PHPStdLib>>
function msgfmt_get_pattern(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
): string;
<<__PHPStdLib>>
function msgfmt_parse(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  string $source,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_parse_message(
  string $locale,
  string $pattern,
  string $source,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function msgfmt_set_pattern(
  HH\FIXME\MISSING_PARAM_TYPE $mf,
  string $pattern,
): bool;
<<__PHPStdLib>>
function normalizer_is_normalized(
  string $input,
  int $form = Normalizer::FORM_C,
): bool;
<<__PHPStdLib>>
function normalizer_normalize(
  string $input,
  int $form = Normalizer::FORM_C,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_create(
  HH\FIXME\MISSING_PARAM_TYPE $locale,
  HH\FIXME\MISSING_PARAM_TYPE $style,
  HH\FIXME\MISSING_PARAM_TYPE $pattern = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_format(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $num,
  HH\FIXME\MISSING_PARAM_TYPE $type = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_format_currency(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $num,
  HH\FIXME\MISSING_PARAM_TYPE $currency,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_attribute(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $attr,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_error_code(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
): int;
<<__PHPStdLib>>
function numfmt_get_error_message(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
): string;
<<__PHPStdLib>>
function numfmt_get_locale(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $type = null,
): string;
<<__PHPStdLib>>
function numfmt_get_pattern(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_symbol(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $attr,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_get_text_attribute(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $attr,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_parse(
  HH\FIXME\MISSING_PARAM_TYPE $formatter,
  HH\FIXME\MISSING_PARAM_TYPE $string,
  HH\FIXME\MISSING_PARAM_TYPE $type,
  inout HH\FIXME\MISSING_PARAM_TYPE $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_parse_currency(
  HH\FIXME\MISSING_PARAM_TYPE $formatter,
  HH\FIXME\MISSING_PARAM_TYPE $string,
  inout HH\FIXME\MISSING_PARAM_TYPE $currency,
  inout HH\FIXME\MISSING_PARAM_TYPE $position,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function numfmt_set_attribute(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $attr,
  HH\FIXME\MISSING_PARAM_TYPE $value,
): bool;
<<__PHPStdLib>>
function numfmt_set_pattern(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
): bool;
<<__PHPStdLib>>
function numfmt_set_symbol(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $attr,
  HH\FIXME\MISSING_PARAM_TYPE $symbol,
): bool;
<<__PHPStdLib>>
function numfmt_set_text_attribute(
  HH\FIXME\MISSING_PARAM_TYPE $nf,
  HH\FIXME\MISSING_PARAM_TYPE $attr,
  HH\FIXME\MISSING_PARAM_TYPE $value,
): bool;
<<__PHPStdLib>>
function resourcebundle_count(
  HH\FIXME\MISSING_PARAM_TYPE $bundle,
): int;
<<__PHPStdLib>>
function resourcebundle_create(
  HH\FIXME\MISSING_PARAM_TYPE $locale,
  HH\FIXME\MISSING_PARAM_TYPE $bundlename,
  bool $fallback = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_get(
  HH\FIXME\MISSING_PARAM_TYPE $bundle,
  HH\FIXME\MISSING_PARAM_TYPE $index,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function resourcebundle_get_error_code(
  HH\FIXME\MISSING_PARAM_TYPE $bundle,
): int;
<<__PHPStdLib>>
function resourcebundle_get_error_message(
  HH\FIXME\MISSING_PARAM_TYPE $bundle,
): string;
<<__PHPStdLib>>
function resourcebundle_locales(
  string $bundlename,
): HH\FIXME\MISSING_RETURN_TYPE;

class Collator {
  const int SORT_REGULAR;
  const int SORT_NUMERIC;
  const int SORT_STRING;
  const int FRENCH_COLLATION;
  const int ALTERNATE_HANDLING;
  const int CASE_FIRST;
  const int CASE_LEVEL;
  const int NORMALIZATION_MODE;
  const int STRENGTH;
  const int HIRAGANA_QUATERNARY_MODE;
  const int NUMERIC_COLLATION;
  const int DEFAULT_VALUE;
  const int PRIMARY;
  const int SECONDARY;
  const int TERTIARY;
  const int DEFAULT_STRENGTH;
  const int QUATERNARY;
  const int IDENTICAL;
  const int OFF;
  const int ON;
  const int SHIFTED;
  const int NON_IGNORABLE;
  const int LOWER_FIRST;
  const int UPPER_FIRST;

  public function __construct(string $locale);
  public function asort(
    inout dynamic $arr,
    int $sort_flag = Collator::SORT_REGULAR,
  ): bool;
  public function compare(
    HH\FIXME\MISSING_PARAM_TYPE $str1,
    HH\FIXME\MISSING_PARAM_TYPE $str2,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  static public function create(string $locale): Collator;
  public function getAttribute(int $attr): int;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public function getLocale(int $type = 0): string;
  public function getSortKey(string $str): HH\FIXME\MISSING_RETURN_TYPE;
  public function getStrength(): int;
  public function setAttribute(
    int $attr,
    int $val,
  ): bool;
  public function setStrength(int $strength): bool;
  public function sortWithSortKeys(
    inout HH\FIXME\MISSING_PARAM_TYPE $arr,
  ): bool;
  public function sort(
    inout dynamic $arr,
    int $sort_flag = Collator::SORT_REGULAR,
  ): bool;
}

class Locale {
  const int ACTUAL_LOCALE;
  const int VALID_LOCALE;
  const string DEFAULT_LOCALE;
  const string LANG_TAG;
  const string EXTLANG_TAG;
  const string SCRIPT_TAG;
  const string REGION_TAG;
  const string VARIANT_TAG;
  const string GRANDFATHERED_LANG_TAG;
  const string PRIVATE_TAG;

  public static function acceptFromHttp(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function canonicalize(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function composeLocale(
    HH\FIXME\MISSING_PARAM_TYPE $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function filterMatches(
    HH\FIXME\MISSING_PARAM_TYPE $arg1,
    HH\FIXME\MISSING_PARAM_TYPE $arg2,
    HH\FIXME\MISSING_PARAM_TYPE $arg3,
  ): bool;
  public static function getAllVariants(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getDefault(): string;
  public static function getDisplayLanguage(
    string $arg1,
    string $arg2,
  ): string;
  public static function getDisplayName(
    string $arg1,
    string $arg2,
  ): string;
  public static function getDisplayRegion(
    string $arg1,
    string $arg2,
  ): string;
  public static function getDisplayScript(
    string $arg1,
    string $arg2,
  ): string;
  public static function getDisplayVariant(
    string $arg1,
    string $arg2,
  ): string;
  public static function getKeywords(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getPrimaryLanguage(
    string $arg1,
  ): string;
  public static function getRegion(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getScript(string $arg1): HH\FIXME\MISSING_RETURN_TYPE;
  public static function lookup(
    HH\FIXME\MISSING_PARAM_TYPE $arg1,
    string $arg2,
    bool $arg3,
    string $arg4,
  ): string;
  public static function parseLocale(
    string $arg1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function setDefault(string $arg1): bool;
}

class Normalizer {
  const int NONE;
  const int FORM_D;
  const int NFD;
  const int FORM_KD;
  const int NFKD;
  const int FORM_C;
  const int NFC;
  const int FORM_KC;
  const int NFKC;

  static public function isNormalized(
    string $input,
    int $form = Normalizer::FORM_C,
  ): bool;
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
  ): ?MessageFormatter;
  public function format(
    HH\FIXME\MISSING_PARAM_TYPE $args,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function formatMessage(
    string $locale,
    string $pattern,
    HH\FIXME\MISSING_PARAM_TYPE $args,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public function getLocale(): string;
  public function getPattern(): string;
  public function parse(string $source): HH\FIXME\MISSING_RETURN_TYPE;
  public static function parseMessage(
    string $locale,
    string $pattern,
    string $args,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setPattern(string $pattern): bool;
}

class IntlDateFormatter {
  const int FULL;
  const int LONG;
  const int MEDIUM;
  const int SHORT;
  const int NONE;
  const int GREGORIAN;
  const int TRADITIONAL;

  public function __construct(
    string $locale,
    int $datetype,
    int $timetype,
    HH\FIXME\MISSING_PARAM_TYPE $timezone = null,
    HH\FIXME\MISSING_PARAM_TYPE $calendar = null,
    string $pattern = "",
  );
  public static function create(
    HH\FIXME\MISSING_PARAM_TYPE $locale,
    HH\FIXME\MISSING_PARAM_TYPE $datetype,
    HH\FIXME\MISSING_PARAM_TYPE $timetype,
    HH\FIXME\MISSING_PARAM_TYPE $timezone = null,
    HH\FIXME\MISSING_PARAM_TYPE $calendar = null,
    HH\FIXME\MISSING_PARAM_TYPE $pattern = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function format(
    HH\FIXME\MISSING_PARAM_TYPE $args = null,
  ): string;
  public static function FormatObject(
    HH\FIXME\MISSING_PARAM_TYPE $object,
    HH\FIXME\MISSING_PARAM_TYPE $format = null,
    HH\FIXME\MISSING_PARAM_TYPE $locale = null,
  ): string;
  public function getCalendar(): int;
  public function getCalendarObject(): IntlCalendar;
  public function getDateType(): int;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public function getLocale(): string;
  public function getPattern(): string;
  public function getTimeType(): int;
  public function getTimezone(): IntlTimeZone;
  public function getTimezoneId(): string;
  public function isLenient(): bool;
  public function localtime(
    string $string,
    inout HH\FIXME\MISSING_PARAM_TYPE $position,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function parse(string $string): HH\FIXME\MISSING_RETURN_TYPE;
  public function parseWithPosition(
    string $string,
    inout HH\FIXME\MISSING_PARAM_TYPE $position,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setCalendar(
    HH\FIXME\MISSING_PARAM_TYPE $which,
  ): bool;
  public function setLenient(bool $lenient): bool;
  public function setPattern(string $pattern): bool;
  public function setTimezone(
    HH\FIXME\MISSING_PARAM_TYPE $zone,
  ): bool;
  public function setTimezoneId(string $zone): bool;
}

class ResourceBundle<T> implements Traversable<T> {
  public function __construct(
    HH\FIXME\MISSING_PARAM_TYPE $locale,
    HH\FIXME\MISSING_PARAM_TYPE $bundlename,
    bool $fallback = true,
  );
  public function count(): int;
  public static function create(
    HH\FIXME\MISSING_PARAM_TYPE $locale,
    HH\FIXME\MISSING_PARAM_TYPE $bundlename,
    bool $fallback = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function get(
    HH\FIXME\MISSING_PARAM_TYPE $index,
    bool $fallback = true,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public static function getLocales(
    string $bundlename,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}

class IntlTimeZone {
  const int DISPLAY_SHORT;
  const int DISPLAY_LONG;
  const int DISPLAY_SHORT_GENERIC;
  const int DISPLAY_LONG_GENERIC;
  const int DISPLAY_SHORT_GMT;
  const int DISPLAY_LONG_GMT;
  const int DISPLAY_SHORT_COMMONLY_USED;
  const int DISPLAY_GENERIC_LOCATION;
  const int TYPE_ANY;
  const int TYPE_CANONICAL;
  const int TYPE_CANONICAL_LOCATION;

  private function __construct();
  public static function countEquivalentIDs(
    string $zoneId,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createDefault(): IntlTimeZone;
  public static function createEnumeration(
    HH\FIXME\MISSING_PARAM_TYPE $countryOrRawOffset = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createTimeZone(
    string $zoneId,
  ): IntlTimeZone;
  public static function createTimeZoneIDEnumeration(
    int $zoneType,
    string $region = "",
    HH\FIXME\MISSING_PARAM_TYPE $rawOffset = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function fromDateTimeZone(
    HH\FIXME\MISSING_PARAM_TYPE $zoneId,
  ): IntlTimeZone;
  public static function getCanonicalID(
    string $zoneId,
    inout HH\FIXME\MISSING_PARAM_TYPE $isSystemID,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDSTSavings(): int;
  public function getDisplayName(
    bool $isDaylight = false,
    int $style = IntlTimeZone::DISPLAY_LONG,
    string $locale = "",
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getEquivalentID(
    string $zoneId,
    int $index,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public static function getGMT(): IntlTimeZone;
  public function getID(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getOffset(
    float $date,
    bool $local,
    inout int $rawOffset,
    inout int $dstOffset,
  ): bool;
  public function getRawOffset(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getRegion(
    string $zoneId,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getTZDataVersion(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getUnknown(): IntlTimeZone;
  public function hasSameRules(
    IntlTimeZone $otherTimeZone,
  ): bool;
  public function toDateTimeZone(): DateTimeZone;
  public function useDaylightTime(): bool;
}

class IntlCalendar {
  const int FIELD_ERA;
  const int FIELD_YEAR;
  const int FIELD_MONTH;
  const int FIELD_WEEK_OF_YEAR;
  const int FIELD_WEEK_OF_MONTH;
  const int FIELD_DATE;
  const int FIELD_DAY_OF_YEAR;
  const int FIELD_DAY_OF_WEEK;
  const int FIELD_DAY_OF_WEEK_IN_MONTH;
  const int FIELD_AM_PM;
  const int FIELD_HOUR;
  const int FIELD_HOUR_OF_DAY;
  const int FIELD_MINUTE;
  const int FIELD_SECOND;
  const int FIELD_MILLISECOND;
  const int FIELD_ZONE_OFFSET;
  const int FIELD_DST_OFFSET;
  const int FIELD_YEAR_WOY;
  const int FIELD_DOW_LOCAL;
  const int FIELD_EXTENDED_YEAR;
  const int FIELD_JULIAN_DAY;
  const int FIELD_MILLISECONDS_IN_DAY;
  const int FIELD_IS_LEAP_MONTH;
  const int FIELD_FIELD_COUNT;
  const int FIELD_DAY_OF_MONTH;
  const int DOW_SUNDAY;
  const int DOW_MONDAY;
  const int DOW_TUESDAY;
  const int DOW_WEDNESDAY;
  const int DOW_THURSDAY;
  const int DOW_FRIDAY;
  const int DOW_SATURDAY;
  const int DOW_TYPE_WEEKDAY;
  const int DOW_TYPE_WEEKEND;
  const int DOW_TYPE_WEEKEND_OFFSET;
  const int DOW_TYPE_WEEKEND_CEASE;
  const int WALLTIME_FIRST;
  const int WALLTIME_LAST;
  const int WALLTIME_NEXT_VALID;

  private function __construct();
  public function add(int $field, int $amount): bool;
  public function after(IntlCalendar $calendar): bool;
  public function before(IntlCalendar $calendar): bool;
  public function clear(
    HH\FIXME\MISSING_PARAM_TYPE $field = null,
  ): bool;
  public static function createInstance(
    HH\FIXME\MISSING_PARAM_TYPE $timeZone = null,
    string $locale = "",
  ): IntlCalendar;
  public function equals(IntlCalendar $calendar): bool;
  public function fieldDifference(
    HH\FIXME\MISSING_PARAM_TYPE $when,
    int $field,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function fromDateTime(
    HH\FIXME\MISSING_PARAM_TYPE $dateTime,
  ): IntlCalendar;
  public function get(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public function getActualMaximum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public function getActualMinimum(int $field): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getAvailableLocales(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDayOfWeekType(
    int $dayOfWeek,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
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
  public static function getNow(): float;
  public function getRepeatedWallTimeOption(): int;
  public function getSkippedWallTimeOption(): int;
  public function getTime(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getTimezone(): IntlTimeZone;
  public function getType(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getWeekendTransition(
    int $dayOfWeek,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function inDaylightTime(): bool;
  public function isEquivalentTo(
    IntlCalendar $calendar,
  ): bool;
  public function isLenient(): bool;
  public function isSet(
    HH\FIXME\MISSING_PARAM_TYPE $field,
  ): bool;
  public function isWeekend(
    HH\FIXME\MISSING_PARAM_TYPE $date = null,
  ): bool;
  public function roll(
    int $field,
    HH\FIXME\MISSING_PARAM_TYPE $amountOrUpOrDown,
  ): bool;
  public function set(
    int $fieldOrYear,
    int $valueOrMonth,
    HH\FIXME\MISSING_PARAM_TYPE $dayOfMonth = null,
    HH\FIXME\MISSING_PARAM_TYPE $hour = null,
    HH\FIXME\MISSING_PARAM_TYPE $minute = null,
    HH\FIXME\MISSING_PARAM_TYPE $second = null,
  ): bool;
  public function setFirstDayOfWeek(
    int $dayOfWeek,
  ): bool;
  public function setLenient(bool $isLenient): bool;
  public function setMinimalDaysInFirstWeek(
    int $numberOfDays,
  ): bool;
  public function setRepeatedWallTimeOption(
    int $wallTimeOption,
  ): bool;
  public function setSkippedWallTimeOption(
    int $wallTimeOption,
  ): bool;
  public function setTime(
    HH\FIXME\MISSING_PARAM_TYPE $date,
  ): bool;
  public function setTimezone(
    HH\FIXME\MISSING_PARAM_TYPE $timeZone,
  ): bool;
  public function toDateTime(): DateTime;
}

class IntlGregorianCalendar extends IntlCalendar {
  public function getGregorianChange(): float;
  public function isLeapYear(int $year): bool;
  public function setGregorianChange(float $date): bool;
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
  const int DONE;
  const int WORD_NONE;
  const int WORD_NONE_LIMIT;
  const int WORD_NUMBER;
  const int WORD_LETTER;
  const int WORD_KANA;
  const int WORD_KANA_LIMIT;
  const int WORD_IDEO;
  const int WORD_IDEO_LIMIT;
  const int LINE_SOFT;
  const int LINE_SOFT_LIMIT;
  const int LINE_HARD;
  const int LINE_HARD_LIMIT;
  const int SENTENCE_TERM;
  const int SENTENCE_SEP;
  const int WORD_NUMBER_LIMIT;
  const int WORD_LETTER_LIMIT;
  const int SENTENCE_TERM_LIMIT;
  const int SENTENCE_SEP_LIMIT;

  // Methods
  public static function createCharacterInstance(
    HH\FIXME\MISSING_PARAM_TYPE $locale = null,
  ): IntlBreakIterator;
  public static function createCodePointInstance(): IntlCodePointBreakIterator;
  public static function createLineInstance(
    HH\FIXME\MISSING_PARAM_TYPE $locale = null,
  ): IntlBreakIterator;
  public static function createSentenceInstance(
    HH\FIXME\MISSING_PARAM_TYPE $locale = null,
  ): IntlBreakIterator;
  public static function createTitleInstance(
    HH\FIXME\MISSING_PARAM_TYPE $locale = null,
  ): IntlBreakIterator;
  public static function createWordInstance(
    HH\FIXME\MISSING_PARAM_TYPE $locale = null,
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
  public function next(
    HH\FIXME\MISSING_PARAM_TYPE $offset = null,
  ): mixed; // returns int or false
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
