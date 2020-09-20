<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int INTL_MAX_LOCALE_LEN = 80;
const string INTL_ICU_VERSION = '52.1';
const string INTL_ICU_DATA_VERSION = '52.1';

const int INTL_IDNA_VARIANT_2003 = 0;
const int INTL_IDNA_VARIANT_UTS46 = 1;

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
const int U_IDNA_ACE_PREFIX_ERROR = 66564;
const int U_IDNA_CHECK_BIDI_ERROR = 66562;
const int U_IDNA_DOMAIN_NAME_TOO_LONG_ERROR = 66568;
const int U_IDNA_ERROR_LIMIT = 66569;
const int U_IDNA_ERROR_START = 66560;
const int U_IDNA_LABEL_TOO_LONG_ERROR = 66566;
const int U_IDNA_PROHIBITED_ERROR = 66560;
const int U_IDNA_STD3_ASCII_RULES_ERROR = 66563;
const int U_IDNA_UNASSIGNED_ERROR = 66561;
const int U_IDNA_VERIFICATION_ERROR = 66565;
const int U_IDNA_ZERO_LENGTH_LABEL_ERROR = 66567;
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
const int U_MULTIPLE_DECIMAL_SEPARATORS = 65793;
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

const int GRAPHEME_EXTR_COUNT = 0;
const int GRAPHEME_EXTR_MAXBYTES = 0;
const int GRAPHEME_EXTR_MAXCHARS = 0;

<<__PHPStdLib>>
function intl_get_error_code();
<<__PHPStdLib>>
function intl_get_error_message();
<<__PHPStdLib>>
function intl_error_name(int $error_code);
<<__PHPStdLib>>
function intl_is_failure(int $error_code);
<<__PHPStdLib>>
function collator_asort($obj, inout $arr, int $sort_flag = Collator::SORT_REGULAR);
<<__PHPStdLib>>
function collator_compare($obj, $str1, $str2);
<<__PHPStdLib>>
function collator_create(string $locale);
<<__PHPStdLib>>
function collator_get_attribute($obj, int $attr);
<<__PHPStdLib>>
function collator_get_error_code($obj);
<<__PHPStdLib>>
function collator_get_error_message($obj);
<<__PHPStdLib>>
function collator_get_locale($obj, int $type = 0);
<<__PHPStdLib>>
function collator_get_sort_key($obj, string $str);
<<__PHPStdLib>>
function collator_get_strength($obj);
<<__PHPStdLib>>
function collator_set_attribute($obj, int $attr, int $val);
<<__PHPStdLib>>
function collator_set_strength($obj, int $strength);
<<__PHPStdLib>>
function collator_sort_with_sort_keys($obj, inout $arr);
<<__PHPStdLib>>
function collator_sort($obj, inout $arr, int $sort_flag = Collator::SORT_REGULAR);
<<__PHPStdLib>>
function idn_to_ascii(string $domain, int $options = 0, int $variant = 0);
<<__PHPStdLib>>
function idn_to_unicode(string $domain, int $options = 0, int $variant = 0);
<<__PHPStdLib>>
function idn_to_utf8(string $domain, int $options = 0, int $variant = 0);
<<__PHPStdLib>>
function datefmt_create($locale, $date_type, $time_type, $timezone_str = null, $calendar = null, $pattern = null);
<<__PHPStdLib>>
function datefmt_format($args = null, $array = null);
<<__PHPStdLib>>
function datefmt_format_object($object, $format = null, $locale = null);
<<__PHPStdLib>>
function datefmt_get_calendar($mf);
<<__PHPStdLib>>
function datefmt_get_calendar_object($mf);
<<__PHPStdLib>>
function datefmt_get_datetype($mf);
<<__PHPStdLib>>
function datefmt_get_error_code($nf);
<<__PHPStdLib>>
function datefmt_get_error_message($coll);
<<__PHPStdLib>>
function datefmt_get_locale($mf);
<<__PHPStdLib>>
function datefmt_get_pattern($mf);
<<__PHPStdLib>>
function datefmt_get_timetype($mf);
<<__PHPStdLib>>
function datefmt_get_timezone($mf);
<<__PHPStdLib>>
function datefmt_get_timezone_id($mf);
<<__PHPStdLib>>
function datefmt_is_lenient($mf);
<<__PHPStdLib>>
function datefmt_localtime($formatter, $string, inout $position);
<<__PHPStdLib>>
function datefmt_parse($formatter, $string, inout $position);
<<__PHPStdLib>>
function datefmt_set_calendar($mf, $calendar);
<<__PHPStdLib>>
function datefmt_set_lenient($mf);
<<__PHPStdLib>>
function datefmt_set_pattern($mf, $pattern);
<<__PHPStdLib>>
function datefmt_set_timezone($mf, $timezone);
<<__PHPStdLib>>
function datefmt_set_timezone_id($mf, $timezone);
<<__PHPStdLib>>
function grapheme_extract(string $haystack, int $size, int $extract_type,
                          int $start, inout ?int $next);
<<__PHPStdLib>>
function grapheme_stripos(string $haystack, string $needle, int $offset = 0);
<<__PHPStdLib>>
function grapheme_stristr(string $haystack, string $needle, bool $before_needle = false);
<<__PHPStdLib>>
function grapheme_strlen(string $string);
<<__PHPStdLib>>
function grapheme_strpos(string $haystack, string $needle, int $offset = 0);
<<__PHPStdLib>>
function grapheme_strripos(string $haystack, string $needle, int $offset = 0);
<<__PHPStdLib>>
function grapheme_strrpos(string $haystack, string $needle, int $offset = 0);
<<__PHPStdLib>>
function grapheme_strstr(string $haystack, string $needle, bool $before_needle = false);
<<__PHPStdLib>>
function grapheme_substr(string $string, int $start, $length = null);
<<__PHPStdLib>>
function intlcal_add(IntlCalendar $calendar, int $field, int $amount);
<<__PHPStdLib>>
function intlcal_after(IntlCalendar $calendar, IntlCalendar $otherCalendar);
<<__PHPStdLib>>
function intlcal_before(IntlCalendar $calendar, IntlCalendar $otherCalendar);
<<__PHPStdLib>>
function intlcal_clear(IntlCalendar $calendar, $field = null);
<<__PHPStdLib>>
function intlcal_create_instance($timeZone = null, string $locale = "");
<<__PHPStdLib>>
function intlcal_equals(IntlCalendar $calendar, IntlCalendar $otherCalendar);
<<__PHPStdLib>>
function intlcal_field_difference(IntlCalendar $calendar, $when, int $field);
<<__PHPStdLib>>
function intlcal_from_date_time($dateTime);
<<__PHPStdLib>>
function intlcal_get(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_get_actual_maximum(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_get_actual_minimum(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_get_available_locales();
<<__PHPStdLib>>
function intlcal_get_day_of_week_type(IntlCalendar $calendar, int $dayOfWeek);
<<__PHPStdLib>>
function intlcal_get_error_code(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_error_message(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_first_day_of_week(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_greatest_minimum(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_get_keyword_values_for_locale(string $key, string $locale, bool $commonlyUsed);
<<__PHPStdLib>>
function intlcal_get_least_maximum(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_get_locale(IntlCalendar $calendar, int $localeType);
<<__PHPStdLib>>
function intlcal_get_maximum(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_get_minimal_days_in_first_week(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_minimum(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_get_now();
<<__PHPStdLib>>
function intlcal_get_repeated_wall_time_option(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_skipped_wall_time_option(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_time(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_time_zone(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_type(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_get_weekend_transition(IntlCalendar $calendar, int $dayOfWeek);
<<__PHPStdLib>>
function intlcal_in_daylight_time(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_is_equivalent_to(IntlCalendar $calendar, IntlCalendar $otherCalendar);
<<__PHPStdLib>>
function intlcal_is_lenient(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlcal_is_set(IntlCalendar $calendar, int $field);
<<__PHPStdLib>>
function intlcal_is_weekend(IntlCalendar $calendar, $date = null);
<<__PHPStdLib>>
function intlcal_roll(IntlCalendar $calendar, int $field, $amountOrUpOrDown = null);
<<__PHPStdLib>>
function intlcal_set(IntlCalendar $calendar, int $fieldOrYear, int $valueOrMonth, $dayOfMonth = null, $hour = null, $minute = null, $second = null);
<<__PHPStdLib>>
function intlcal_set_first_day_of_week(IntlCalendar $calendar, int $dayOfWeek);
<<__PHPStdLib>>
function intlcal_set_lenient(IntlCalendar $calendar, bool $isLenient);
<<__PHPStdLib>>
function intlcal_set_minimal_days_in_first_week(IntlCalendar $calendar, int $numberOfDays);
<<__PHPStdLib>>
function intlcal_set_repeated_wall_time_option(IntlCalendar $calendar, int $wallTimeOption);
<<__PHPStdLib>>
function intlcal_set_skipped_wall_time_option(IntlCalendar $calendar, int $wallTimeOption);
<<__PHPStdLib>>
function intlcal_set_time(IntlCalendar $calendar, $date);
<<__PHPStdLib>>
function intlcal_set_time_zone(IntlCalendar $calendar, $timeZone);
<<__PHPStdLib>>
function intlcal_to_date_time(IntlCalendar $calendar);
<<__PHPStdLib>>
function intlgregcal_create_instance($timeZoneOrYear = null, $localeOrMonth = null, $dayOfMonth = null, $hour = null, $minute = null, $second = null);
<<__PHPStdLib>>
function intlgregcal_get_gregorian_change(IntlGregorianCalendar $calendar);
<<__PHPStdLib>>
function intlgregcal_is_leap_year(IntlGregorianCalendar $calendar, int $year);
<<__PHPStdLib>>
function intlgregcal_set_gregorian_change(IntlGregorianCalendar $calendar, float $date);
<<__PHPStdLib>>
function intltz_count_equivalent_ids(string $zoneId);
<<__PHPStdLib>>
function intltz_create_default();
<<__PHPStdLib>>
function intltz_create_enumeration($countryOrRawOffset = null);
<<__PHPStdLib>>
function intltz_create_time_zone(string $zoneId);
<<__PHPStdLib>>
function intltz_create_time_zone_id_enumeration(int $zoneType, string $region = "", $rawOffset = null);
<<__PHPStdLib>>
function intltz_from_date_time_zone(DateTimeZone $dateTimeZone);
<<__PHPStdLib>>
function intltz_get_canonical_id(string $zoneId, inout $isSystemID);
<<__PHPStdLib>>
function intltz_get_display_name(IntlTimeZone $timeZone, bool $isDaylight = false, int $style = IntlTimeZone::DISPLAY_LONG, string $locale = "");
<<__PHPStdLib>>
function intltz_get_dst_savings(IntlTimeZone $timeZone);
<<__PHPStdLib>>
function intltz_get_equivalent_id(string $zoneId, int $index);
<<__PHPStdLib>>
function intltz_get_error_code(IntlTimeZone $timeZone);
<<__PHPStdLib>>
function intltz_get_error_message(IntlTimeZone $timeZone);
<<__PHPStdLib>>
function intltz_get_gmt();
<<__PHPStdLib>>
function intltz_get_id(IntlTimeZone $timeZone);
<<__PHPStdLib>>
function intltz_get_offset(IntlTimeZone $timeZone, float $date, bool $local, inout int $rawOffset, inout int $dstOffset);
<<__PHPStdLib>>
function intltz_get_raw_offset(IntlTimeZone $timeZone);
<<__PHPStdLib>>
function intltz_get_region(string $zoneId);
<<__PHPStdLib>>
function intltz_get_tz_data_version();
<<__PHPStdLib>>
function intltz_get_unknown();
<<__PHPStdLib>>
function intltz_has_same_rules(IntlTimeZone $timeZone, ?IntlTimeZone $otherTimeZone = null);
<<__PHPStdLib>>
function intltz_to_date_time_zone(IntlTimeZone $timeZone);
<<__PHPStdLib>>
function intltz_use_daylight_time(IntlTimeZone $timeZone);
<<__PHPStdLib>>
function locale_accept_from_http(string $arg1);
<<__PHPStdLib>>
function locale_canonicalize(string $arg1);
<<__PHPStdLib>>
function locale_compose($arg1);
<<__PHPStdLib>>
function locale_filter_matches($arg1, $arg2, $arg3);
<<__PHPStdLib>>
function locale_get_all_variants(string $arg1);
<<__PHPStdLib>>
function locale_get_default();
<<__PHPStdLib>>
function locale_get_display_language(string $arg1, string $arg2);
<<__PHPStdLib>>
function locale_get_display_name(string $arg1, string $arg2);
<<__PHPStdLib>>
function locale_get_display_region(string $arg1, string $arg2);
<<__PHPStdLib>>
function locale_get_display_script(string $arg1, string $arg2);
<<__PHPStdLib>>
function locale_get_display_variant(string $arg1, string $arg2);
<<__PHPStdLib>>
function locale_get_keywords(string $arg1);
<<__PHPStdLib>>
function locale_get_primary_language(string $arg1);
<<__PHPStdLib>>
function locale_get_region(string $arg1);
<<__PHPStdLib>>
function locale_get_script(string $arg1);
<<__PHPStdLib>>
function locale_lookup($arg1, string $arg2, bool $arg3, string $arg4);
<<__PHPStdLib>>
function locale_parse(string $arg1);
<<__PHPStdLib>>
function locale_set_default(string $arg1);
<<__PHPStdLib>>
function msgfmt_create(string $locale, string $pattern);
<<__PHPStdLib>>
function msgfmt_format($nf, $args);
<<__PHPStdLib>>
function msgfmt_format_message(string $locale, string $pattern, $args);
<<__PHPStdLib>>
function msgfmt_get_error_code($nf);
<<__PHPStdLib>>
function msgfmt_get_error_message($coll);
<<__PHPStdLib>>
function msgfmt_get_locale($mf);
<<__PHPStdLib>>
function msgfmt_get_pattern($mf);
<<__PHPStdLib>>
function msgfmt_parse($nf, string $source);
<<__PHPStdLib>>
function msgfmt_parse_message(string $locale, string $pattern, string $source);
<<__PHPStdLib>>
function msgfmt_set_pattern($mf, string $pattern);
<<__PHPStdLib>>
function normalizer_is_normalized(string $input, int $form = Normalizer::FORM_C);
<<__PHPStdLib>>
function normalizer_normalize(string $input, int $form = Normalizer::FORM_C);
<<__PHPStdLib>>
function numfmt_create($locale, $style, $pattern = null);
<<__PHPStdLib>>
function numfmt_format($nf, $num, $type = null);
<<__PHPStdLib>>
function numfmt_format_currency($nf, $num, $currency);
<<__PHPStdLib>>
function numfmt_get_attribute($nf, $attr);
<<__PHPStdLib>>
function numfmt_get_error_code($nf);
<<__PHPStdLib>>
function numfmt_get_error_message($nf);
<<__PHPStdLib>>
function numfmt_get_locale($nf, $type = null);
<<__PHPStdLib>>
function numfmt_get_pattern($nf);
<<__PHPStdLib>>
function numfmt_get_symbol($nf, $attr);
<<__PHPStdLib>>
function numfmt_get_text_attribute($nf, $attr);
<<__PHPStdLib>>
function numfmt_parse($formatter, $string, $type, inout $position);
<<__PHPStdLib>>
function numfmt_parse_currency($formatter, $string, inout $currency,
                               inout $position);
<<__PHPStdLib>>
function numfmt_set_attribute($nf, $attr, $value);
<<__PHPStdLib>>
function numfmt_set_pattern($nf, $pattern);
<<__PHPStdLib>>
function numfmt_set_symbol($nf, $attr, $symbol);
<<__PHPStdLib>>
function numfmt_set_text_attribute($nf, $attr, $value);
<<__PHPStdLib>>
function resourcebundle_count($bundle);
<<__PHPStdLib>>
function resourcebundle_create($locale, $bundlename, bool $fallback = false);
<<__PHPStdLib>>
function resourcebundle_get($bundle, $index);
<<__PHPStdLib>>
function resourcebundle_get_error_code($bundle);
<<__PHPStdLib>>
function resourcebundle_get_error_message($bundle);
<<__PHPStdLib>>
function resourcebundle_locales(string $bundlename);

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

  public function __construct(string $locale);
  public function asort(inout $arr, int $sort_flag = Collator::SORT_REGULAR);
  public function compare($str1, $str2);
  static public function create(string $locale);
  public function getAttribute(int $attr);
  public function getErrorCode();
  public function getErrorMessage();
  public function getLocale(int $type = 0);
  public function getSortKey(string $str);
  public function getStrength();
  public function setAttribute(int $attr, int $val);
  public function setStrength(int $strength);
  public function sortWithSortKeys(inout $arr);
  public function sort(inout $arr, int $sort_flag = Collator::SORT_REGULAR);
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

  public static function acceptFromHttp(string $arg1);
  public static function canonicalize(string $arg1);
  public static function composeLocale($arg1);
  public static function filterMatches($arg1, $arg2, $arg3);
  public static function getAllVariants(string $arg1);
  public static function getDefault();
  public static function getDisplayLanguage(string $arg1, string $arg2);
  public static function getDisplayName(string $arg1, string $arg2);
  public static function getDisplayRegion(string $arg1, string $arg2);
  public static function getDisplayScript(string $arg1, string $arg2);
  public static function getDisplayVariant(string $arg1, string $arg2);
  public static function getKeywords(string $arg1);
  public static function getPrimaryLanguage(string $arg1);
  public static function getRegion(string $arg1);
  public static function getScript(string $arg1);
  public static function lookup($arg1, string $arg2, bool $arg3, string $arg4);
  public static function parseLocale(string $arg1);
  public static function setDefault(string $arg1);
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

  static public function isNormalized(string $input, int $form = Normalizer::FORM_C);
  static public function normalize(string $input, int $form = Normalizer::FORM_C);
}

class MessageFormatter {
  public function __construct(string $locale, string $pattern);
  public static function create(string $locale, string $pattern);
  public function format($args);
  public static function formatMessage(string $locale, string $pattern, $args);
  public function getErrorCode();
  public function getErrorMessage();
  public function getLocale();
  public function getPattern();
  public function parse(string $source);
  public static function parseMessage(string $locale, string $pattern, string $args);
  public function setPattern(string $pattern);
}

class IntlDateFormatter {
  const int FULL = 0;
  const int LONG = 1;
  const int MEDIUM = 2;
  const int SHORT = 3;
  const int NONE = -1;
  const int GREGORIAN = 1;
  const int TRADITIONAL = 0;

  public function __construct(string $locale, int $datetype, int $timetype, $timezone = null, $calendar = null, string $pattern = "");
  public static function create($locale, $datetype, $timetype, $timezone = null, $calendar = null, $pattern = null);
  public function format($args = null);
  public static function formatObject($object, $format = null, $locale = null);
  public function getCalendar();
  public function getCalendarObject();
  public function getDateType();
  public function getErrorCode();
  public function getErrorMessage();
  public function getLocale();
  public function getPattern();
  public function getTimeType();
  public function getTimeZone();
  public function getTimeZoneId();
  public function isLenient();
  public function localtime(string $string, inout $position);
  public function parse(string $string);
  public function parseWithPosition(string $string, inout $position);
  public function setCalendar($which);
  public function setLenient(bool $lenient);
  public function setPattern(string $pattern);
  public function setTimeZone($zone);
  public function setTimeZoneId(string $zone);
}

class ResourceBundle<T> implements Traversable<T> {
  public function __construct($locale, $bundlename, bool $fallback = true);
  public function count();
  public static function create($locale, $bundlename, bool $fallback = false);
  public function get($index, bool $fallback = true);
  public function getErrorCode();
  public function getErrorMessage();
  public static function getLocales(string $bundlename);
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
  public static function countEquivalentIDs(string $zoneId);
  public static function createDefault();
  public static function createEnumeration($countryOrRawOffset = null);
  public static function createTimeZone(string $zoneId);
  public static function createTimeZoneIDEnumeration(int $zoneType, string $region = "", $rawOffset = null);
  public static function fromDateTimeZone($zoneId);
  public static function getCanonicalID(string $zoneId, inout $isSystemID);
  public function getDSTSavings();
  public function getDisplayName(bool $isDaylight = false, int $style = IntlTimeZone::DISPLAY_LONG, string $locale = "");
  public static function getEquivalentID(string $zoneId, int $index);
  public function getErrorCode();
  public function getErrorMessage();
  public static function getGMT();
  public function getID();
  public function getOffset(float $date, bool $local, inout int $rawOffset, inout int $dstOffset);
  public function getRawOffset();
  public static function getRegion(string $zoneId);
  public static function getTZDataVersion();
  public static function getUnknown();
  public function hasSameRules(IntlTimeZone $otherTimeZone);
  public function toDateTimeZone();
  public function useDaylightTime();
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
  public function add(int $field, int $amount);
  public function after(IntlCalendar $calendar);
  public function before(IntlCalendar $calendar);
  public function clear($field = null);
  public static function createInstance($timeZone = null, string $locale = "");
  public function equals(IntlCalendar $calendar);
  public function fieldDifference($when, int $field);
  public static function fromDateTime($dateTime);
  public function get(int $field);
  public function getActualMaximum(int $field);
  public function getActualMinimum(int $field);
  public static function getAvailableLocales();
  public function getDayOfWeekType(int $dayOfWeek);
  public function getErrorCode();
  public function getErrorMessage();
  public function getFirstDayOfWeek();
  public function getGreatestMinimum(int $field);
  public static function getKeywordValuesForLocale(string $key, string $locale, bool $commonlyUsed);
  public function getLeastMaximum(int $field);
  public function getLocale(int $localeType);
  public function getMaximum(int $field);
  public function getMinimalDaysInFirstWeek();
  public function getMinimum(int $field);
  public static function getNow();
  public function getRepeatedWallTimeOption();
  public function getSkippedWallTimeOption();
  public function getTime();
  public function getTimeZone();
  public function getType();
  public function getWeekendTransition(int $dayOfWeek);
  public function inDaylightTime();
  public function isEquivalentTo(IntlCalendar $calendar);
  public function isLenient();
  public function isSet($field);
  public function isWeekend($date = null);
  public function roll(int $field, $amountOrUpOrDown);
  public function set(int $fieldOrYear, int $valueOrMonth, $dayOfMonth = null, $hour = null, $minute = null, $second = null);
  public function setFirstDayOfWeek(int $dayOfWeek);
  public function setLenient(bool $isLenient);
  public function setMinimalDaysInFirstWeek(int $numberOfDays);
  public function setRepeatedWallTimeOption(int $wallTimeOption);
  public function setSkippedWallTimeOption(int $wallTimeOption);
  public function setTime($date);
  public function setTimeZone($timeZone);
  public function toDateTime();
  public function call__($fname, $args): mixed;
}

class IntlGregorianCalendar extends IntlCalendar {
  public function getGregorianChange();
  public function isLeapYear(int $year);
  public function setGregorianChange(float $date);
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
