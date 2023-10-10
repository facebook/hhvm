<?hh

/**
 * Date Formatter is a concrete class that enables locale-dependent
 * formatting/parsing of dates using pattern strings and/or canned patterns.
 * This class represents the ICU date formatting functionality. It allows
 * users to display dates in a localized format or to parse strings into PHP
 * date values using pattern strings and/or canned patterns.
 */
<<__NativeData>>
class IntlDateFormatter {
  /**
   * Create a date formatter
   *
   * @param string $locale - Locale to use when formatting or parsing or
   *   NULL to use the value specified in the ini setting
   *   intl.default_locale.
   * @param int $datetype - Date type to use (none, short, medium, long,
   *   full). This is one of the IntlDateFormatter constants. It can also
   *   be NULL, in which case ICU's default date type will be used.
   * @param int $timetype - Time type to use (none, short, medium, long,
   *   full). This is one of the IntlDateFormatter constants. It can also
   *   be NULL, in which case ICU's default time type will be used.
   * @param mixed $timezone - Time zone ID. The default (and the one used
   *   if NULL is given) is the one returned by date_default_timezone_get()
   *   or, if applicable, that of the IntlCalendar object passed for the
   *   calendar parameter. This ID must be a valid identifier on ICU's
   *   database or an ID representing an explicit offset, such as
   *   GMT-05:30.   This can also be an IntlTimeZone or a DateTimeZone
   *   object.
   * @param mixed $calendar - Calendar to use for formatting or parsing.
   *   The default value is NULL, which corresponds to
   *   IntlDateFormatter::GREGORIAN. This can either be one of the
   *   IntlDateFormatter calendar constants or an IntlCalendar. Any
   *   IntlCalendar object passed will be clone; it will not be changed by
   *   the IntlDateFormatter. This will determine the calendar type used
   *   (gregorian, islamic, persian, etc.) and, if NULL is given for the
   *   timezone parameter, also the timezone used.
   * @param string $pattern - Optional pattern to use when formatting or
   *   parsing. Possible patterns are documented at .
   *
   * @return IntlDateFormatter - The created IntlDateFormatter or FALSE
   *   in case of failure.
   */
  <<__Native>>
  public function __construct(string $locale, int $datetype, int $timetype,
                              mixed $timezone = NULL, mixed $calendar = NULL,
                              string $pattern = ''): void;

  public static function create(string $locale, int $datetype, int $timetype,
                                mixed $timezone = NULL, mixed $calendar = NULL,
                                string $pattern = ''): mixed {
    try {
      return new IntlDateFormatter($locale, $datetype, $timetype,
                                   $timezone, $calendar, $pattern);
    } catch (Exception $e) {
      return false;
    }
  }

  /**
   * Format the date/time value as a string
   *
   * @param mixed $value - Value to format. This may be a DateTime
   *   object, an IntlCalendar object, a numeric type representing a
   *   (possibly fractional) number of seconds since epoch or an array in
   *   the format output by localtime().   If a DateTime or an IntlCalendar
   *   object is passed, its timezone is not considered. The object will be
   *   formatted using the formater's configured timezone. If one wants to
   *   use the timezone of the object to be formatted,
   *   IntlDateFormatter::setTimezone() must be called before with the
   *   object's timezone. Alternatively, the static function
   *   IntlDateFormatter::formatObject() may be used instead.
   *
   * @return string - The formatted string or, if an error occurred,
   *   FALSE.
   */
  <<__Native>>
  public function format(mixed $value): string;

  /**
   * Formats an object
   *
   * @param object $object -
   * @param mixed $format -
   * @param string $locale -
   *
   * @return string - A string with result.
   */
  <<__Native>>
  public static function FormatObject(object $obj,
                                      mixed $format = NULL,
                                      ?string $locale = NULL): string;

  /**
   * Get the calendar type used for the IntlDateFormatter
   *
   * @return int - The calendar type being used by the formatter. Either
   *   IntlDateFormatter::TRADITIONAL or IntlDateFormatter::GREGORIAN.
   */
  <<__Native>>
  public function getCalendar(): int;

  /**
   * Get the datetype used for the IntlDateFormatter
   *
   * @return int - The current date type value of the formatter.
   */
  <<__Native>>
  public function getDateType(): int;

  /**
   * Get the error code from last operation
   *
   * @return int - The error code, one of UErrorCode values. Initial
   *   value is U_ZERO_ERROR.
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get the error text from the last operation.
   *
   * @return string - Description of the last error.
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Get the locale used by formatter
   *
   * @param int $which - Default ULOC_ACTUAL_LOCALE
   *
   * @return string - the locale of this formatter or 'false' if error
   */
  <<__Native>>
  public function getLocale(?int $which = null): string;

  /**
   * Get the pattern used for the IntlDateFormatter
   *
   * @return string - The pattern string being used to format/parse.
   */
  <<__Native>>
  public function getPattern(): string;

  /**
   * Get the timetype used for the IntlDateFormatter
   *
   * @return int - The current date type value of the formatter.
   */
  <<__Native>>
  public function getTimeType(): int;

  /**
   * Get the timezone-id used for the IntlDateFormatter
   *
   * @return string - ID string for the time zone used by this
   *   formatter.
   */
  <<__Native>>
  public function getTimezoneId(): string;

  /**
   * Get copy of formatter's calendar object
   *
   * @return IntlCalendar - A copy of the internal calendar object used
   *   by this formatter.
   */
  <<__Native>>
  public function getCalendarObject(): IntlCalendar;

  /**
   * Get formatter's timezone
   *
   * @return IntlTimeZone - The associated IntlTimeZone object.
   */
  <<__Native>>
  public function getTimezone(): IntlTimeZone;

  /**
   * Get the lenient used for the IntlDateFormatter
   *
   * @return bool - TRUE if parser is lenient, FALSE if parser is strict.
   *   By default the parser is lenient.
   */
  <<__Native>>
  public function isLenient(): bool;

  /**
   * Parse string to a field-based time value
   *
   * @param string $value - string to convert to a time
   * @param int $position - Position at which to start the parsing in
   *   $value (zero-based). If no error occurs before $value is consumed,
   *   $parse_pos will contain -1 otherwise it will contain the position at
   *   which parsing ended . If $parse_pos > strlen($value), the parse
   *   fails immediately.
   *
   * @return array - Localtime compatible array of integers : contains 24
   *   hour clock value in tm_hour field
   */
  <<__Native>>
  public function localtime(string $value,
                            inout mixed $position): mixed;

  /**
   * Parse string to a timestamp value
   *
   * @param string $value - string to convert to a time
   * @param int $position - Position at which to start the parsing in
   *   $value (zero-based). If no error occurs before $value is consumed,
   *   $parse_pos will contain -1 otherwise it will contain the position at
   *   which parsing ended (and the error occurred). This variable will
   *   contain the end position if the parse fails. If $parse_pos >
   *   strlen($value), the parse fails immediately.
   *
   * @return int - timestamp parsed value, or FALSE if value can't be
   *   parsed.
   */
  public function parse(string $value): mixed {
    $position = null;
    $result = $this->parseWithPosition($value, inout $position);
    return $result;
  }

  <<__Native>>
  public function parseWithPosition(string $value,
                                    inout mixed $position): mixed;

  /**
   * Sets the calendar type used by the formatter
   *
   * @param mixed $which - This can either be: the calendar type to use
   *   (default is IntlDateFormatter::GREGORIAN, which is also used if NULL
   *   is specified) or an IntlCalendar object.   Any IntlCalendar object
   *   passed in will be cloned; no modifications will be made to the
   *   argument object.   The timezone of the formatter will only be kept
   *   if an IntlCalendar object is not passed, otherwise the new timezone
   *   will be that of the passed object.
   *
   * @return bool -
   */
  <<__Native>>
  public function setCalendar(mixed $which): bool;

  /**
   * Set the leniency of the parser
   *
   * @param bool $lenient - Sets whether the parser is lenient or not,
   *   default is TRUE (lenient).
   *
   * @return bool -
   */
  <<__Native>>
  public function setLenient(bool $lenient): bool;

  /**
   * Set the pattern used for the IntlDateFormatter
   *
   * @param string $pattern - New pattern string to use. Possible
   *   patterns are documented at .
   *
   * @return bool - Bad formatstrings are usually the cause of the
   *   failure.
   */
  <<__Native>>
  public function setPattern(string $pattern): bool;

  /**
   * Sets the time zone to use
   *
   * @param string $zone - The time zone ID string of the time zone to
   *   use. If NULL or the empty string, the default time zone for the
   *   runtime is used.
   *
   * @return bool -
   */
  public function setTimezoneId(string $zone): bool {
    trigger_error("Use datefmt_set_timezone() instead, which also accepts ".
                  "a plain time zone identifier and for which this function ".
                  "is now an alias", E_DEPRECATED);
    return $this->setTimezone($zone);
  }

  /**
   * Sets formatter's timezone
   *
   * @param mixed $zone -
   *
   * @return bool - Returns TRUE on success and FALSE on failure.
   */
  <<__Native>>
  public function setTimezone(mixed $zone): bool;

}

/**
 * Create a date formatter
 *
 * @param string $locale - Locale to use when formatting or parsing or
 *   NULL to use the value specified in the ini setting
 *   intl.default_locale.
 * @param int $datetype - Date type to use (none, short, medium, long,
 *   full). This is one of the IntlDateFormatter constants. It can also be
 *   NULL, in which case ICU's default date type will be used.
 * @param int $timetype - Time type to use (none, short, medium, long,
 *   full). This is one of the IntlDateFormatter constants. It can also be
 *   NULL, in which case ICU's default time type will be used.
 * @param mixed $timezone - Time zone ID. The default (and the one used
 *   if NULL is given) is the one returned by date_default_timezone_get()
 *   or, if applicable, that of the IntlCalendar object passed for the
 *   calendar parameter. This ID must be a valid identifier on ICU's
 *   database or an ID representing an explicit offset, such as GMT-05:30.
 *    This can also be an IntlTimeZone or a DateTimeZone object.
 * @param mixed $calendar - Calendar to use for formatting or parsing.
 *   The default value is NULL, which corresponds to
 *   IntlDateFormatter::GREGORIAN. This can either be one of the
 *   IntlDateFormatter calendar constants or an IntlCalendar. Any
 *   IntlCalendar object passed will be clone; it will not be changed by
 *   the IntlDateFormatter. This will determine the calendar type used
 *   (gregorian, islamic, persian, etc.) and, if NULL is given for the
 *   timezone parameter, also the timezone used.
 * @param string $pattern - Optional pattern to use when formatting or
 *   parsing. Possible patterns are documented at .
 *
 * @return IntlDateFormatter - The created IntlDateFormatter or FALSE in
 *   case of failure.
 */
function datefmt_create($locale,
                        $datetype,
                        $timetype,
                        $timezone = NULL,
                        $calendar = NULL,
                        $pattern = ''): mixed {
  return IntlDateFormatter::create($locale, $datetype, $timetype,
                                   $timezone, $calendar, $pattern);
}

/**
 * Format the date/time value as a string
 *
 * @param intldateformatter $fmt - The date formatter resource.
 * @param mixed $value - Value to format. This may be a DateTime object,
 *   an IntlCalendar object, a numeric type representing a (possibly
 *   fractional) number of seconds since epoch or an array in the format
 *   output by localtime().   If a DateTime or an IntlCalendar object is
 *   passed, its timezone is not considered. The object will be formatted
 *   using the formater's configured timezone. If one wants to use the
 *   timezone of the object to be formatted,
 *   IntlDateFormatter::setTimezone() must be called before with the
 *   object's timezone. Alternatively, the static function
 *   IntlDateFormatter::formatObject() may be used instead.
 *
 * @return string - The formatted string or, if an error occurred,
 *   FALSE.
 */
function datefmt_format(IntlDateFormatter $fmt,
                        $value): string {
  return $fmt->format($value);
}

/**
 * Formats an object
 *
 * @param object $object -
 * @param mixed $format -
 * @param string $locale -
 *
 * @return string - A string with result.
 */
function datefmt_format_object($obj,
                               $format = NULL,
                               $locale = NULL): string {
  return IntlDateFormatter::FormatObject($obj, $format, $locale);
}

/**
 * Get the calendar type used for the IntlDateFormatter
 *
 * @param intldateformatter $fmt - The formatter resource
 *
 * @return int - The calendar type being used by the formatter. Either
 *   IntlDateFormatter::TRADITIONAL or IntlDateFormatter::GREGORIAN.
 */
function datefmt_get_calendar(IntlDateFormatter $fmt): int {
  return $fmt->getCalendar();
}

/**
 * Get the datetype used for the IntlDateFormatter
 *
 * @param intldateformatter $fmt - The formatter resource.
 *
 * @return int - The current date type value of the formatter.
 */
function datefmt_get_datetype(IntlDateFormatter $fmt): int {
  return $fmt->getDateType();
}

/**
 * Get the error code from last operation
 *
 * @param intldateformatter $fmt - The formatter resource.
 *
 * @return int - The error code, one of UErrorCode values. Initial value
 *   is U_ZERO_ERROR.
 */
function datefmt_get_error_code(IntlDateFormatter $fmt): int {
  return $fmt->getErrorCode();
}

/**
 * Get the error text from the last operation.
 *
 * @param intldateformatter $fmt - The formatter resource.
 *
 * @return string - Description of the last error.
 */
function datefmt_get_error_message(IntlDateFormatter $fmt): string {
  return $fmt->getErrorMessage();
}

/**
 * Get the locale used by formatter
 *
 * @param intldateformatter $fmt - The formatter resource
 * @param int $which -
 *
 * @return string - the locale of this formatter or 'false' if error
 */
function datefmt_get_locale(IntlDateFormatter $fmt, ?int $which = null) {
  return $fmt->getLocale($which);
}

/**
 * Get the pattern used for the IntlDateFormatter
 *
 * @param intldateformatter $fmt - The formatter resource.
 *
 * @return string - The pattern string being used to format/parse.
 */
function datefmt_get_pattern(IntlDateFormatter $fmt): string {
  return $fmt->getPattern();
}

/**
 * Get the timetype used for the IntlDateFormatter
 *
 * @param intldateformatter $fmt - The formatter resource.
 *
 * @return int - The current date type value of the formatter.
 */
function datefmt_get_timetype(IntlDateFormatter $fmt): int {
  return $fmt->getTimeType();
}

/**
 * Get the timezone-id used for the IntlDateFormatter
 *
 * @param intldateformatter $fmt - The formatter resource.
 *
 * @return string - ID string for the time zone used by this formatter.
 */
function datefmt_get_timezone_id(IntlDateFormatter $fmt): string {
  return $fmt->getTimezoneId();
}

/**
 * Get copy of formatter's calendar object
 *
 * @return IntlCalendar - A copy of the internal calendar object used by
 *   this formatter.
 */
function datefmt_get_calendar_object(IntlDateFormatter $fmt): IntlCalendar {
  return $fmt->getCalendarObject();
}

/**
 * Get formatter's timezone
 *
 * @return IntlTimeZone - The associated IntlTimeZone object.
 */
function datefmt_get_timezone(IntlDateFormatter $fmt): IntlTimeZone {
  return $fmt->getTimezone();
}

/**
 * Get the lenient used for the IntlDateFormatter
 *
 * @param intldateformatter $fmt - The formatter resource.
 *
 * @return bool - TRUE if parser is lenient, FALSE if parser is strict.
 *   By default the parser is lenient.
 */
function datefmt_is_lenient(IntlDateFormatter $fmt): bool {
  return $fmt->isLenient();
}

/**
 * Parse string to a field-based time value
 *
 * @param intldateformatter $fmt - The formatter resource
 * @param string $value - string to convert to a time
 * @param int $position - Position at which to start the parsing in
 *   $value (zero-based). If no error occurs before $value is consumed,
 *   $parse_pos will contain -1 otherwise it will contain the position at
 *   which parsing ended . If $parse_pos > strlen($value), the parse fails
 *   immediately.
 *
 * @return array - Localtime compatible array of integers : contains 24
 *   hour clock value in tm_hour field
 */
function datefmt_localtime(IntlDateFormatter $fmt,
                           $value,
                           inout $position): mixed {
  $result = $fmt->localtime($value, inout $position);
  return $result;
}

/**
 * Parse string to a timestamp value
 *
 * @param intldateformatter $fmt - The formatter resource
 * @param string $value - string to convert to a time
 * @param int $position - Position at which to start the parsing in
 *   $value (zero-based). If no error occurs before $value is consumed,
 *   $parse_pos will contain -1 otherwise it will contain the position at
 *   which parsing ended (and the error occurred). This variable will
 *   contain the end position if the parse fails. If $parse_pos >
 *   strlen($value), the parse fails immediately.
 *
 * @return int - timestamp parsed value, or FALSE if value can't be
 *   parsed.
 */
function datefmt_parse(IntlDateFormatter $fmt,
                       $value,
                       inout $position): mixed {
  $result = $fmt->parseWithPosition($value, inout $position);
  return $result;
}

/**
 * Sets the calendar type used by the formatter
 *
 * @param intldateformatter $fmt - The formatter resource.
 * @param mixed $which - This can either be: the calendar type to use
 *   (default is IntlDateFormatter::GREGORIAN, which is also used if NULL
 *   is specified) or an IntlCalendar object.   Any IntlCalendar object
 *   passed in will be cloned; no modifications will be made to the
 *   argument object.   The timezone of the formatter will only be kept if
 *   an IntlCalendar object is not passed, otherwise the new timezone will
 *   be that of the passed object.
 *
 * @return bool -
 */
function datefmt_set_calendar(IntlDateFormatter $fmt,
                              $which): bool {
  return $fmt->setCalendar($which);
}

/**
 * Set the leniency of the parser
 *
 * @param intldateformatter $fmt - The formatter resource
 * @param bool $lenient - Sets whether the parser is lenient or not,
 *   default is TRUE (lenient).
 *
 * @return bool -
 */
function datefmt_set_lenient(IntlDateFormatter $fmt,
                             $lenient): bool {
  return $fmt->setLenient($lenient);
}

/**
 * Set the pattern used for the IntlDateFormatter
 *
 * @param intldateformatter $fmt - The formatter resource.
 * @param string $pattern - New pattern string to use. Possible patterns
 *   are documented at .
 *
 * @return bool - Bad formatstrings are usually the cause of the
 *   failure.
 */
function datefmt_set_pattern(IntlDateFormatter $fmt,
                             $pattern): bool {
  return $fmt->setPattern($pattern);
}

/**
 * Sets the time zone to use
 *
 * @param intldateformatter $fmt - The formatter resource.
 * @param string $zone - The time zone ID string of the time zone to use.
 *   If NULL or the empty string, the default time zone for the runtime is
 *   used.
 *
 * @return bool -
 */
function datefmt_set_timezone_id(IntlDateFormatter $fmt,
                                 $zone): bool {
  return $fmt->setTimezoneId($zone);
}

/**
 * Sets formatter's timezone
 *
 * @param mixed $zone -
 *
 * @return bool - Returns TRUE on success and FALSE on failure.
 */
function datefmt_set_timezone(IntlDateFormatter $fmt, $zone): bool {
  return $fmt->setTimezone($zone);
}
