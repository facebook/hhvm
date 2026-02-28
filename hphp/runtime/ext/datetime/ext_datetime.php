<?hh

/**
 * Representation of date and time.
 *
 */
<<__NativeData>>
class DateTime implements DateTimeInterface {

  /**
   * Add an interval to a datetime object
   *
   * @param DateInterval $interval - DateInterval object containing the time to
   *   add.
   *
   * @return DateTime - Returns the DateTime object for method chaining
   *
   */
  <<__Native>>
  public function add(DateInterval $interval)[write_props]: mixed;

  <<__Native>>
  public function __construct(
    string $time = "now",
    ?DateTimeZone $timezone = null,
  )[read_globals]: void;

  /**
   * Parse a date according to a format and create a DateTime object
   *
   * @param string $format - DateTime format specifier
   * @param string $time - Date and time to parse
   * @param DateTimeZone $timezone - DateTimeZone for the given time
   *
   * @return mixed - Returns a new DateTime instance or FALSE on failure.
   *
   */
  <<__Native>>
  public static function createFromFormat(
    string $format,
    string $time,
    ?DateTimeZone $timezone = null,
  )[read_globals]: mixed;

  /**
   * Find the interval between two DateTime objects
   *
   * @param DateTimeInterface $datetime2 - DateTime object to compare agains
   * @param bool $absolute - Whether to return absolute difference
   *
   * @return DateInterval - Returns a DateInterval object representing the
   *   distance between two times
   *
   */
  <<__Native>>
  public function diff(mixed $datetime2, mixed $absolute = false)[]: mixed;

  /**
   * Returns date formatted according to given format.
   *
   * @param string $format - DateTime object returned by date_create()
   *
   * @return string - Returns the formatted date string on success or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function format(mixed $format)[]: string;

  /**
   * Returns the last errors encountered by the datetime extension
   *
   * @return array - Vector of error messages
   *
   */
  <<__Native>>
  public static function getLastErrors()[read_globals]: shape(
    'warning_count' => int,
    'warnings' => dict<int, string>,
    'error_count' => int,
    'errors' => dict<int, string>
  );

  /**
   * Returns the timezone offset.
   *
   * @return int - Returns the timezone offset in seconds from UTC on success
   *   or FALSE on failure.
   *
   */
  <<__Native>>
  public function getOffset()[]: int;

  /**
   * Returns the unix timestamp representing the date.
   *
   * @return int - Epoch representing the datetime object
   *
   */
  <<__Native>>
  public function getTimestamp()[]: int;

  /**
   * Return time zone relative to given DateTime.
   *
   * @return mixed - Returns a DateTimeZone object on success or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function getTimezone()[]: mixed;

  /**
   * Alter the timestamp of a DateTime object by incrementing or
   *   decrementing in a format accepted by strtotime().
   *
   * @param string $modify - DateTime object returned by date_create(). The
   *   function modifies this object.
   *
   * @return DateTime - Returns the modified DateTime object or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function modify(string $modify)[write_props]: mixed;

  /**
   * Resets the current date of the DateTime object to a different date.
   *
   * @param int $year - DateTime object returned by date_create(). The
   *   function modifies this object.
   * @param int $month - Year of the date.
   * @param int $day - Month of the date.
   *
   * @return DateTime - Returns the modified DateTime object or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function setDate(int $year, int $month, int $day)[write_props]: DateTime;

  /**
   * Set a date according to the ISO 8601  standard - using weeks and day
   *   offsets rather than specific dates.
   *
   * @param int $year - DateTime object returned by date_create(). The
   *   function modifies this object.
   * @param int $week - Year of the date.
   * @param int $day - Week of the date.
   *
   * @return DateTime - Returns the modified DateTime object or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function setISODate(int $year, int $week, int $day = 1)[write_props]: DateTime;

  /**
   * Resets the current time of the DateTime object to a different time.
   *
   * @param int $hour - DateTime object returned by date_create(). The
   *   function modifies this object.
   * @param int $minute - Hour of the time.
   * @param int $second - Minute of the time.
   *
   * @return DateTime - Returns the modified DateTime object or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function setTime(int $hour, int $minute, int $second = 0)[write_props]: DateTime;

  /**
   * Set the DateTime object according to the timestamp provided
   *
   * @param int $unixtimestamp - Unix timestamp to update the DateTime object
   *   to.
   *
   * @return DateTime - Returns the DateTime object for method chaining
   *
   */
  <<__Native>>
  public function setTimestamp(int $unixtimestamp)[read_globals, write_props]: DateTime;

  /**
   * @param DateTimeZone $timezone - DateTime object returned by date_create().
   *   The function modifies this object.
   *
   * @return DateTime - Returns the modified DateTime object or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function setTimezone(DateTimeZone $timezone)[write_props]: mixed;

  /**
   * Subtract an interval from a datetime object
   *
   * @param DateInterval $interval - DateInterval object containing the time to
   *   subtract.
   *
   * @return DateTime - Returns the DateTime object for method chaining
   *
   */
  <<__Native>>
  public function sub(DateInterval $interval)[write_props]: mixed;

  <<__Native>>
  public function __sleep()[write_props]: varray<string>;

  <<__Native>>
  public function __wakeup()[]: void;

  <<__Native>>
  public function __debugInfo()[]: darray<string, mixed>;

}

/**
 * Representation of time zone.
 *
 */
<<__NativeData>>
class DateTimeZone {

  /**
   * @param string $timezone
   *
   */
  <<__Native>>
  public function __construct(string $timezone)[]: void;

  /**
   * Returns location information for a timezone
   *
   * @return array - Array containing location information about timezone.
   *
   */
  <<__Native>>
  public function getLocation()[]: shape(
    'country_code' => string,
    'latitude' => float,
    'longitude' => float,
    'comments' => string,
  );

  /**
   * Returns the name of the timezone.
   *
   * @return string - One of timezones.
   *
   */
  <<__Native>>
  public function getName()[]: string;

  /**
   * This function returns the offset to GMT for the date/time specified in the
   *   datetime parameter. The GMT offset is calculated with the timezone
   *   information contained in the DateTimeZone object being used.
   *
   * @param DateTimeInterface $datetime - DateTimeZone object returned by
   *   timezone_open()
   *
   * @return int - Returns time zone offset in seconds on success or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function getOffset(DateTimeInterface $datetime)[]: mixed;

  /**
   * @return array - Returns numerically indexed array containing associative
   *   array with all transitions on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function getTransitions(int $timestamp_begin = PHP_INT_MIN,
                          int $timestamp_end = PHP_INT_MAX)[]: mixed;


  const type TAbbrev = shape(
    'dst' => bool,
    'offset' => float,
    'timezone_id' => ?string,
  );
  /**
   * @return array - Returns array on success or FALSE on failure.
   *
   */
  <<__Native>>
  public static function listAbbreviations(
  )[]: dict<string, vec<this::TAbbrev>>;

  /**
   * @param int $what - One of DateTimeZone class constants.
   * @param string $country - The timestamp which is used as a base for the
   *   calculation of relative dates.
   *
   * @return mixed - Returns array on success or FALSE on failure.
   *
   */
  <<__Native>>
  public static function listIdentifiers(int $what = 2047,
                                  string $country = "")[]: mixed;

  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

/**
 * Represents a date interval.
 *
 */
<<__NativeData>>
class DateInterval {

  /**
   * Creates a new DateInterval object
   *
   * @param string $interval_spec
   *
   */
  <<__Native>>
  public function __construct(string $interval_spec): void;

  /**
   * Sets up a DateInterval from the relative parts of the string
   *
   * @param string $time - A date with relative parts. Specifically, the
   *   relative formats supported by the parser used for strtotime() and
   *   DateTime will be used to construct the DateInterval.
   *
   * @return DateInterval - Returns a new DateInterval instance.
   *
   */
  <<__Native>>
  public static function createFromDateString(string $time): DateInterval;

  /**
   * Formats the interval
   *
   * @param string $format - DateInterval format specifier.
   *
   * @return string - Returns the formatted interval.
   *
   */
  <<__Native>>
  public function format(string $format): string;
}

/**
 * Checks the validity of the date formed by the arguments. A date is
 *   considered valid if each parameter is properly defined.
 *
 * @param int $month - The month is between 1 and 12 inclusive.
 * @param int $day - The day is within the allowed number of days for the
 *   given month. Leap years are taken into consideration.
 * @param int $year - The year is between 1 and 32767 inclusive.
 *
 * @return bool - Returns TRUE if the date given is valid; otherwise returns
 *   FALSE.
 *
 */
<<__Native>>
function checkdate(int $month, int $day, int $year): bool;

function date_add(DateTime $datetime, DateInterval $interval): mixed {
  return $datetime->add($interval);
}

function date_create_from_format(string $format,
                                 string $time,
                                 ?DateTimeZone $timezone = null): mixed {
  return DateTime::createFromFormat($format, $time, $timezone);
}

<<__Native>>
function date_parse_from_format(string $format, string $date): mixed;

<<__Native>>
function date_create(?string $time = null,
                     ?DateTimeZone $timezone = null): mixed;

function date_date_set(DateTime $datetime,
                       int $year,
                       int $month,
                       int $day): void {
  $datetime->setDate($year, $month, $day);
}

/**
 * In order of preference, this function returns the default timezone by:
 *   Reading the timezone set using the date_default_timezone_set() function (if
 *   any)  Reading the TZ environment variable (if non empty) (Prior to PHP
 *   5.3.0)  Reading the value of the date.timezone ini option (if set)
 *   Querying the host operating system (if supported and allowed by the OS)  If
 *   none of the above succeed, date_default_timezone_get() will return a
 *   default timezone of UTC.
 *
 * @return string - Returns a string.
 *
 */
<<__Native>>
function date_default_timezone_get(): string;

/**
 * date_default_timezone_set() sets the default timezone used by all date/time
 *   functions.  Since PHP 5.1.0 (when the date/time functions were rewritten),
 *   every call to a date/time function will generate a E_NOTICE if the timezone
 *   isn't valid, and/or a E_WARNING message if using the system settings or the
 *   TZ environment variable.  Instead of using this function to set the default
 *   timezone in your script, you can also use the INI setting date.timezone to
 *   set the default timezone.
 *
 * @param string $name - The timezone identifier, like UTC or Europe/Lisbon.
 *   The list of valid identifiers is available in the List of Supported
 *   Timezones.
 *
 * @return bool - This function returns FALSE if the timezone_identifier isn't
 *   valid, or TRUE otherwise.
 *
 */
<<__Native>>
function date_default_timezone_set(string $name): bool;

function date_get_last_errors()[read_globals]: shape(
  'warning_count' => int,
  'warnings' => dict<int, string>,
  'error_count' => int,
  'errors' => dict<int, string>
) {
  return DateTime::getLastErrors();
}

function date_interval_create_from_date_string(string $time): DateInterval {
  return DateInterval::createFromDateString($time);
}

function date_interval_format(DateInterval $interval,
                              string $format_spec): string {
  return $interval->format($format_spec);
}

function date_isodate_set(DateTime $datetime,
                          int $year,
                          int $week,
                          int $day = 1): DateTime {
  return $datetime->setISODate($year, $week, $day);
}

<<__Native>>
function date_format(DateTimeInterface $datetime, string $format): mixed;

function date_modify(DateTime $datetime, string $modify): void {
  $datetime->modify($modify);
}

/**
 * @param string $date - Date in format accepted by strtotime().
 *
 * @return mixed - Returns array with information about the parsed date on
 *   success or FALSE on failure.
 *
 */
<<__Native>>
function date_parse(string $date): mixed;

function date_sub(DateTime $datetime, DateInterval $interval): mixed {
  return $datetime->sub($interval);
}

/**
 * @param int $ts - Timestamp.
 * @param float $latitude - Latitude in degrees.
 * @param float $longitude - Longitude in degrees.
 */
<<__Native>>
function date_sun_info(int $ts, float $latitude, float $longitude): shape(
  'sunrise' => mixed,
  'sunset' => mixed,
  'transit' => int,
  'civil_twilight_begin' => mixed,
  'civil_twilight_end' => mixed,
  'nautical_twilight_begin' => mixed,
  'nautical_twilight_end' => mixed,
  'astronomical_twilight_begin' => mixed,
  'astronomical_twilight_end' => mixed,
);

/**
 * date_sunrise() returns the sunrise time for a given day (specified as a
 *   timestamp) and location.
 *
 * @param int $timestamp - The timestamp of the day from which the sunrise
 *   time is taken.
 * @param int $format - format constants constant description example
 *   SUNFUNCS_RET_STRING returns the result as string 16:46 SUNFUNCS_RET_DOUBLE
 *   returns the result as float 16.78243132 SUNFUNCS_RET_TIMESTAMP returns the
 *   result as integer (timestamp) 1095034606
 * @param float $latitude - Defaults to North, pass in a negative value for
 *   South. See also: date.default_latitude
 * @param float $longitude - Defaults to East, pass in a negative value for
 *   West. See also: date.default_longitude
 * @param float $zenith - Default: date.sunrise_zenith
 * @param float $gmt_offset - Specified in hours.
 *
 * @return mixed - Returns the sunrise time in a specified format on success
 *   or FALSE on failure.
 *
 */
<<__Native>>
function date_sunrise(int $timestamp,
                      int $format = SUNFUNCS_RET_STRING,
                      ?float $latitude = null,
                      ?float $longitude = null,
                      ?float $zenith = null,
                      ?float $gmt_offset = null): mixed;

/**
 * date_sunset() returns the sunset time for a given day (specified as a
 *   timestamp) and location.
 *
 * @param int $timestamp - The timestamp of the day from which the sunset time
 *   is taken.
 * @param int $format - format constants constant description example
 *   SUNFUNCS_RET_STRING returns the result as string 16:46 SUNFUNCS_RET_DOUBLE
 *   returns the result as float 16.78243132 SUNFUNCS_RET_TIMESTAMP returns the
 *   result as integer (timestamp) 1095034606
 * @param float $latitude - Defaults to North, pass in a negative value for
 *   South. See also: date.default_latitude
 * @param float $longitude - Defaults to East, pass in a negative value for
 *   West. See also: date.default_longitude
 * @param float $zenith - Default: date.sunset_zenith
 * @param float $gmt_offset - Specified in hours.
 *
 * @return mixed - Returns the sunset time in a specified format on success or
 *   FALSE on failure.
 *
 */
<<__Native>>
function date_sunset(int $timestamp,
                     int $format = SUNFUNCS_RET_STRING,
                     ?float $latitude = null,
                     ?float $longitude = null,
                     ?float $zenith = null,
                     ?float $gmt_offset = null): mixed;

function date_time_set(DateTime $datetime,
                       int $hour,
                       int $minute,
                       int $second = 0): void {
  $datetime->setTime($hour, $minute, $second);
}

function date_timestamp_set(DateTime $datetime, int $timestamp): DateTime {
  return $datetime->setTimestamp($timestamp);
}

function date_timezone_set(DateTime $datetime, DateTimeZone $timezone): mixed {
  return $datetime->setTimezone($timezone);
}

/**
 * Returns a string formatted according to the given format string using the
 *   given integer timestamp or the current time if no timestamp is given. In
 *   other words, timestamp is optional and defaults to the value of time().
 *
 * @return mixed - Returns a formatted date string. If a non-numeric value is
 *   used for timestamp, FALSE is returned and an E_WARNING level error is
 *   emitted.
 *
 */
<<__Native>>
function date(string $format, ?int $timestamp = null): mixed;

/**
 * Returns an associative array containing the date information of the
 *   timestamp, or the current local time if no timestamp is given.
 *
 * @param int $timestamp - The optional timestamp parameter is an integer Unix
 *   timestamp that defaults to the current local time if a timestamp is not
 *   given. In other words, it defaults to the value of time().
 *
 * @return array
 *
 */
<<__Native>>
function getdate(?int $timestamp = null): shape(
  'seconds' => int,
  'minutes' => int,
  'hours' => int,
  'mday' => int,
  'wday' => int,
  'mon' => int,
  'year' => int,
  'yday' => int,
  'weekday' => string,
  'month' => string,
  ...
);

/**
 * This is an interface to gettimeofday(2). It returns an associative array
 *   containing the data returned from the system call.
 *
 * @param bool $return_float - When set to TRUE, a float instead of an array
 *   is returned.
 *
 * @return mixed - By default an array is returned. If return_float is set,
 *   then a float is returned.  Array keys: "sec" - seconds since the Unix Epoch
 *   "usec" - microseconds "minuteswest" - minutes west of Greenwich "dsttime" -
 *   type of dst correction
 *
 */
<<__Native>>
function gettimeofday(bool $return_float = false): mixed;

/**
 * Identical to the date() function except that the time returned is Greenwich
 *   Mean Time (GMT).
 *
 * @param string $format - The format of the outputted date string. See the
 *   formatting options for the date() function.
 * @param int $timestamp - The optional timestamp parameter is an integer Unix
 *   timestamp that defaults to the current local time if a timestamp is not
 *   given. In other words, it defaults to the value of time().
 *
 * @return mixed - Returns a formatted date string. If a non-numeric value is
 *   used for timestamp, FALSE is returned and an E_WARNING level error is
 *   emitted.
 *
 */
<<__Native>>
function gmdate(string $format, ?int $timestamp = null): mixed;

/**
 * Identical to mktime() except the passed parameters represents a GMT date.
 *   gmmktime() internally uses mktime() so only times valid in derived local
 *   time can be used.  Like mktime(), arguments may be left out in order from
 *   right to left, with any omitted arguments being set to the current
 *   corresponding GMT value.
 *
 * @return mixed - Returns a integer Unix timestamp.
 *
 */
<<__Native>>
function gmmktime(int $hour = PHP_INT_MAX,
                  int $minute = PHP_INT_MAX,
                  int $second = PHP_INT_MAX,
                  int $month = PHP_INT_MAX,
                  int $day = PHP_INT_MAX,
                  int $year = PHP_INT_MAX): mixed;

/**
 * Behaves the same as strftime() except that the time returned is Greenwich
 *   Mean Time (GMT). For example, when run in Eastern Standard Time (GMT
 *   -0500), the first line below prints "Dec 31 1998 20:00:00", while the
 *   second prints "Jan 01 1999 01:00:00".
 *
 * @param string $format - See description in strftime().
 * @param int $timestamp - The optional timestamp parameter is an integer Unix
 *   timestamp that defaults to the current local time if a timestamp is not
 *   given. In other words, it defaults to the value of time().
 *
 * @return string - Returns a string formatted according to the given format
 *   string using the given timestamp or the current local time if no timestamp
 *   is given. Month and weekday names and other language dependent strings
 *   respect the current locale set with setlocale().
 *
 */
<<__Native>>
function gmstrftime(string $format, ?int $timestamp = null): mixed;

/**
 * Returns a number formatted according to the given format string using the
 *   given integer timestamp or the current local time if no timestamp is given.
 *   In other words, timestamp is optional and defaults to the value of time().
 *   Unlike the function date(), idate() accepts just one char in the format
 *   parameter.
 *
 * @param string $format - The following characters are recognized in the
 *   format parameter string format character Description B Swatch Beat/Internet
 *   Time d Day of the month h Hour (12 hour format) H Hour (24 hour format) i
 *   Minutes I (uppercase i) returns 1 if DST is activated, 0 otherwise L
 *   (uppercase l) returns 1 for leap year, 0 otherwise m Month number s Seconds
 *   t Days in current month U Seconds since the Unix Epoch - January 1 1970
 *   00:00:00 UTC - this is the same as time() w Day of the week (0 on Sunday) W
 *   ISO-8601 week number of year, weeks starting on Monday y Year (1 or 2
 *   digits - check note below) Y Year (4 digits) z Day of the year Z Timezone
 *   offset in seconds
 * @param int $timestamp - The optional timestamp parameter is an integer Unix
 *   timestamp that defaults to the current local time if a timestamp is not
 *   given. In other words, it defaults to the value of time().
 *
 * @return mixed - Returns an integer.  As idate() always returns an integer
 *   and as they can't start with a "0", idate() may return fewer digits than
 *   you would expect. See the example below.
 *
 */
<<__Native>>
function idate(string $format, ?int $timestamp = null): mixed;

/**
 * The localtime() function returns an array identical to that of the
 *   structure returned by the C function call.
 *
 * @param int $timestamp - The optional timestamp parameter is an integer Unix
 *   timestamp that defaults to the current local time if a timestamp is not
 *   given. In other words, it defaults to the value of time().
 * @param bool $is_associative - If set to FALSE or not supplied then the
 *   array is returned as a regular, numerically indexed array. If the argument
 *   is set to TRUE then localtime() returns an associative array containing all
 *   the different elements of the structure returned by the C function call to
 *   localtime. The names of the different keys of the associative array are as
 *   follows:  "tm_sec" - seconds "tm_min" - minutes "tm_hour" - hour "tm_mday"
 *   - day of the month Months are from 0 (Jan) to 11 (Dec) and days of the week
 *   are from 0 (Sun) to 6 (Sat). "tm_mon" - month of the year, starting with 0
 *   for January "tm_year" - Years since 1900 "tm_wday" - Day of the week
 *   "tm_yday" - Day of the year "tm_isdst" - Is daylight savings time in effect
 *
 * @return array
 *
 */
<<__Native>>
function localtime(?int $timestamp = null,
                   bool $is_associative = false): varray_or_darray<mixed>;

/**
 * microtime() returns the current Unix timestamp with microseconds. This
 *   function is only available on operating systems that support the
 *   gettimeofday() system call.
 *
 * @param bool $get_as_float - When called without the optional argument, this
 *   function returns the string "msec sec" where sec is the current time
 *   measured in the number of seconds since the Unix Epoch (0:00:00 January 1,
 *   1970 GMT), and msec is the microseconds part. Both portions of the string
 *   are returned in units of seconds.  If the optional get_as_float is set to
 *   TRUE then a float (in seconds) is returned.
 *
 * @return mixed
 *
 */
<<__Native>>
function microtime(bool $get_as_float = false)[leak_safe]: mixed;

/**
 * Returns the Unix timestamp corresponding to the arguments given. This
 *   timestamp is a long integer containing the number of seconds between the
 *   Unix Epoch (January 1 1970 00:00:00 GMT) and the time specified.  Arguments
 *   may be left out in order from right to left; any arguments thus omitted
 *   will be set to the current value according to the local date and time.
 *
 * @param int $hour - The number of the hour.
 * @param int $minute - The number of the minute.
 * @param int $second - The number of seconds past the minute.
 * @param int $month - The number of the month.
 * @param int $day - The number of the day.
 * @param int $year - The number of the year, may be a two or four digit
 *   value, with values between 0-69 mapping to 2000-2069 and 70-100 to
 *   1970-2000. On systems where time_t is a 32bit signed integer, as most
 *   common today, the valid range for year is somewhere between 1901 and 2038.
 *   However, before PHP 5.1.0 this range was limited from 1970 to 2038 on some
 *   systems (e.g. Windows).
 *
 * @return mixed - mktime() returns the Unix timestamp of the arguments given.
 *   If the arguments are invalid, the function returns FALSE (before PHP 5.1 it
 *   returned -1).
 *
 */
<<__Native>>
function mktime(int $hour = PHP_INT_MAX,
                int $minute = PHP_INT_MAX,
                int $second = PHP_INT_MAX,
                int $month = PHP_INT_MAX,
                int $day = PHP_INT_MAX,
                int $year = PHP_INT_MAX)[leak_safe]: mixed;

/**
 * Format the time and/or date according to locale settings. Month and weekday
 *   names and other language-dependent strings respect the current locale set
 *   with setlocale().  Not all conversion specifiers may be supported by your C
 *   library, in which case they will not be supported by PHP's strftime().
 *   Additionally, not all platforms support negative timestamps, so your date
 *   range may be limited to no earlier than the Unix epoch. This means that %e,
 *   %T, %R and, %D (and possibly others) - as well as dates prior to Jan 1,
 *   1970 - will not work on Windows, some Linux distributions, and a few other
 *   operating systems. For Windows systems, a complete overview of supported
 *   conversion specifiers can be found at MSDN.
 *
 * @return mixed - Returns a string formatted according format using the given
 *   timestamp or the current local time if no timestamp is given. Month and
 *   weekday names and other language-dependent strings respect the current
 *   locale set with setlocale().
 *
 */
<<__Native>>
function strftime(string $format, ?int $timestamp = null)[leak_safe]: mixed;

/**
 * strptime() returns an array with the date parsed, or FALSE on error.  Month
 *   and weekday names and other language dependent strings respect the current
 *   locale set with setlocale() (LC_TIME).
 *
 * @param string $date - The string to parse (e.g. returned from strftime()).
 * @param string $format - The format used in date (e.g. the same as used in
 *   strftime()). Note that some of the format options available to strftime()
 *   may not have any effect within strptime(); the exact subset that are
 *   supported will vary based on the operating system and C library in use.
 *   For more information about the format options, read the strftime() page.
 *
 * @return mixed - Returns an array or FALSE on failure.  The following
 *   parameters are returned in the array parameters Description "tm_sec"
 *   Seconds after the minute (0-61) "tm_min" Minutes after the hour (0-59)
 *   "tm_hour" Hour since midnight (0-23) "tm_mday" Day of the month (1-31)
 *   "tm_mon" Months since January (0-11) "tm_year" Years since 1900 "tm_wday"
 *   Days since Sunday (0-6) "tm_yday" Days since January 1 (0-365) "unparsed"
 *   the date part which was not recognized using the specified format
 *
 */
<<__Native>>
function strptime(string $date, string $format): mixed;

/**
 * @param string $input - Date and Time Formats.
 * @param int $timestamp - The timestamp which is used as a base for the
 *   calculation of relative dates.
 *
 * @return mixed - Returns a timestamp on success, FALSE otherwise. Previous
 *   to PHP 5.1.0, this function would return -1 on failure.
 *
 */
<<__Native>>
function strtotime(string $input, ?int $timestamp = null)[leak_safe]: mixed;

/**
 * Returns the current time measured in the number of seconds since the Unix
 *   Epoch (January 1 1970 00:00:00 GMT).
 *
 */
<<__Native>>
function time()[leak_safe]: int;

function timezone_abbreviations_list(
): ?dict<string, vec<DateTimeZone::TAbbrev>> {
  return DateTimeZone::listAbbreviations();
}

/**
 * @param int $what - One of DateTimeZone class constants.
 * @param string $country - The timestamp which is used as a base for the
 *   calculation of relative dates.
 *
 * @return mixed
 *
 */
function timezone_identifiers_list(int $what = 2047,
                                   string $country = ""): mixed {
  return DateTimeZone::listIdentifiers($what, $country);
}

function timezone_location_get(DateTimeZone $timezone): ?shape(
  'country_code' => string,
  'latitude' => float,
  'longitude' => float,
  'comments' => string,
) {
  return $timezone->getLocation();
}

/**
 * @param string $abbr - Time zone abbreviation.
 * @param int $gmtoffset - Offset from GMT in seconds. Defaults to -1 which
 *   means that first found time zone corresponding to abbr is returned.
 *   Otherwise exact offset is searched and only if not found then the first
 *   time zone with any offset is returned.
 * @param bool $isdst - Daylight saving time indicator. Defaults to -1, which
 *   means that whether the time zone has daylight saving or not is not taken
 *   into consideration when searching. If this is set to 1, then the gmtOffset
 *   is assumed to be an offset with daylight saving in effect; if 0, then
 *   gmtOffset is assumed to be an offset without daylight saving in effect. If
 *   abbr doesn't exist then the time zone is searched solely by the gmtOffset
 *   and isdst.
 *
 * @return mixed - Returns time zone name on success or FALSE on failure.
 *
 */
<<__Native>>
function timezone_name_from_abbr(string $abbr,
                                 int $gmtoffset = -1,
                                 int $isdst = 1): mixed;

function timezone_name_get(DateTimeZone $timezone): string {
  return $timezone->getName();
}

function timezone_open(string $timezone): mixed {
  try {
    return new DateTimeZone($timezone);
  }
  catch (Exception $e) {
    $msg = str_replace("DateTimeZone::__construct", "timezone_open",
                       $e->getMessage());
    trigger_error(HH\FIXME\UNSAFE_CAST<mixed, string>($msg), E_WARNING);
    return false;
  }
}

function timezone_transitions_get(DateTimeZone $timezone,
                                  int $timestamp_begin = PHP_INT_MIN,
                                  int $timestamp_end = PHP_INT_MAX): mixed {
  return $timezone->getTransitions($timestamp_begin, $timestamp_end);
}

<<__Native>>
function timezone_version_get(): string;
