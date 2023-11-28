<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const string DATE_ATOM;
const string DATE_COOKIE;
const string DATE_ISO8601;
const string DATE_RFC1036;
const string DATE_RFC1123;
const string DATE_RFC2822;
const string DATE_RFC3339;
const string DATE_RFC822;
const string DATE_RFC850;
const string DATE_RSS;
const string DATE_W3C;

const int DAY_1;
const int DAY_2;
const int DAY_3;
const int DAY_4;
const int DAY_5;
const int DAY_6;
const int DAY_7;

type DateTimeErrors = shape(
  'warning_count' => int,
  'warnings' => darray<int, string>,
  'error_count' => int,
  'errors' => darray<int, string>,
);

<<__PHPStdLib>>
function checkdate(
  int $month,
  int $day,
  int $year,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_add(
  HH\FIXME\MISSING_PARAM_TYPE $datetime,
  HH\FIXME\MISSING_PARAM_TYPE $interval,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_create_from_format(
  string $format,
  string $time,
  HH\FIXME\MISSING_PARAM_TYPE $timezone = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_create(
  HH\FIXME\MISSING_PARAM_TYPE $time = null,
  HH\FIXME\MISSING_PARAM_TYPE $timezone = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_date_set(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  int $year,
  int $month,
  int $day,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_default_timezone_get(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_default_timezone_set(string $name): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_diff(
  HH\FIXME\MISSING_PARAM_TYPE $datetime,
  HH\FIXME\MISSING_PARAM_TYPE $datetime2,
  bool $absolute = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_format(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  string $format,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_get_last_errors(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_interval_create_from_date_string(
  string $time,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_interval_format(
  HH\FIXME\MISSING_PARAM_TYPE $interval,
  string $format_spec,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_isodate_set(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  int $year,
  int $week,
  int $day = 1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_modify(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  string $modify,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_offset_get(
  HH\FIXME\MISSING_PARAM_TYPE $object,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_parse(string $date): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_sub(
  HH\FIXME\MISSING_PARAM_TYPE $datetime,
  HH\FIXME\MISSING_PARAM_TYPE $interval,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_sun_info(
  int $ts,
  float $latitude,
  float $longitude,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_sunrise(
  int $timestamp,
  int $format = 0,
  ?float $latitude = null,
  ?float $longitude = null,
  ?float $zenith = null,
  ?float $gmt_offset = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_sunset(
  int $timestamp,
  int $format = 0,
  ?float $latitude = null,
  ?float $longitude = null,
  ?float $zenith = null,
  ?float $gmt_offset = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_time_set(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  int $hour,
  int $minute,
  int $second = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_timestamp_get(
  HH\FIXME\MISSING_PARAM_TYPE $datetime,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_timestamp_set(
  HH\FIXME\MISSING_PARAM_TYPE $datetime,
  int $timestamp,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_timezone_get(
  HH\FIXME\MISSING_PARAM_TYPE $object,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date_timezone_set(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  HH\FIXME\MISSING_PARAM_TYPE $timezone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function date(
  string $format,
  ?int $timestamp = null,
)/*: string*/ : HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function getdate(?int $timestamp = null): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gettimeofday(bool $return_float = false): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gmdate(
  string $format,
  ?int $timestamp = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gmmktime(
  int $hour = PHP_INT_MAX,
  int $minute = PHP_INT_MAX,
  int $second = PHP_INT_MAX,
  int $month = PHP_INT_MAX,
  int $day = PHP_INT_MAX,
  int $year = PHP_INT_MAX,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gmstrftime(
  string $format,
  ?int $timestamp = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function idate(
  string $format,
  ?int $timestamp = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function localtime(
  ?int $timestamp = null,
  bool $is_associative = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function microtime(
  bool $get_as_float = false,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mktime(
  int $hour = PHP_INT_MAX,
  int $minute = PHP_INT_MAX,
  int $second = PHP_INT_MAX,
  int $month = PHP_INT_MAX,
  int $day = PHP_INT_MAX,
  int $year = PHP_INT_MAX,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strftime(
  string $format,
  ?int $timestamp = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strptime(string $date, string $format): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function strtotime(
  string $input,
  ?int $timestamp = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function time()[leak_safe]: int;
<<__PHPStdLib>>
function timezone_abbreviations_list(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_identifiers_list(
  int $what = 2047,
  string $country = '',
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_location_get(
  HH\FIXME\MISSING_PARAM_TYPE $timezone,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_name_from_abbr(
  string $abbr,
  int $gmtoffset = -1,
  int $isdst = 1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_name_get(
  HH\FIXME\MISSING_PARAM_TYPE $object,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_offset_get(
  HH\FIXME\MISSING_PARAM_TYPE $object,
  HH\FIXME\MISSING_PARAM_TYPE $dt,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_open(string $timezone): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_transitions_get(
  DateTimeZone $object,
  int $timestamp_begin = PHP_INT_MIN,
  int $timestamp_end = PHP_INT_MAX,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function timezone_version_get(): HH\FIXME\MISSING_RETURN_TYPE;

interface DateTimeInterface {
  public function diff(
    DateTimeInterface $datetime2,
    bool $absolute = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function format(string $format): HH\FIXME\MISSING_RETURN_TYPE;
  public function getOffset(): ~int;
  public function getTimestamp()[]: ~int;
  public function getTimezone(): HH\FIXME\MISSING_RETURN_TYPE;
}

class DateTime implements DateTimeInterface {
  const string ATOM;
  const string COOKIE;
  const string ISO8601;
  const string RFC822;
  const string RFC850;
  const string RFC1036;
  const string RFC1123;
  const string RFC2822;
  const string RFC3339;
  const string RSS;
  const string W3C;

  public function __construct(
    string $time = 'now',
    ?DateTimeZone $timezone = null,
  )[read_globals];
  public function add(DateInterval $interval)[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
  public function modify(string $modify)[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
  public function getOffset()[]: int;
  public function getTimestamp()[]: int;
  public function getTimezone()[]: HH\FIXME\MISSING_RETURN_TYPE;
  public function setDate(
    int $year,
    int $month,
    int $day,
  )[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
  public function setISODate(
    int $year,
    int $week,
    int $day = 1,
  )[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
  public function setTime(
    int $hour,
    int $minute,
    int $second = 0,
  )[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
  public function setTimestamp(int $unixtimestamp)[read_globals, write_props]: this;
  public function setTimezone(
    DateTimeZone $timezone,
  )[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
  public function sub(DateInterval $interval)[write_props]: HH\FIXME\MISSING_RETURN_TYPE;
  public function diff(
    DateTimeInterface $datetime2,
    bool $absolute = false,
  )[]: HH\FIXME\MISSING_RETURN_TYPE;
  public function format(string $format)[]: HH\FIXME\MISSING_RETURN_TYPE;
  public static function createFromFormat(
    string $format,
    string $time,
    ?DateTimeZone $timezone = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getLastErrors()[read_globals]: DateTimeErrors;
}

class DateTimeImmutable implements DateTimeInterface {
  private DateTime $data;

  public function __construct(
    string $time = 'now',
    ?DateTimeZone $timezone = null,
  );
  public function add(DateInterval $interval): this;
  public function modify(string $modify): this;
  public function getOffset(): int;
  public function getTimestamp()[]: int;
  public function getTimezone(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setDate(int $year, int $month, int $day): this;
  public function setISODate(int $year, int $week, int $day = 1): this;
  public function setTime(int $hour, int $minute, int $second = 0): this;
  public function setTimestamp(int $unixtimestamp): this;
  public function setTimezone(DateTimeZone $timezone): this;
  public function sub(DateInterval $interval): this;
  public function diff(
    DateTimeInterface $datetime2,
    bool $absolute = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function format(string $format): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createFromFormat(
    string $format,
    string $time,
    ?DateTimeZone $timezone = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function createFromMutable(
    DateTime $datetime,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public static function getLastErrors(): DateTimeErrors;
  public function __clone(): HH\FIXME\MISSING_RETURN_TYPE;
}

class DateTimeZone {
  const int AFRICA;
  const int AMERICA;
  const int ANTARCTICA;
  const int ARCTIC;
  const int ASIA;
  const int ATLANTIC;
  const int AUSTRALIA;
  const int EUROPE;
  const int INDIAN;
  const int PACIFIC;
  const int UTC;
  const int ALL;
  const int ALL_WITH_BC;
  const int PER_COUNTRY;
  public function __construct(string $timezone)[];
  public function getLocation()[]: shape(
    'country_code' => string,
    'latitude' => float,
    'longitude' => float,
    'comments' => string,
  ) {}
  public function getName()[]: string {}
  public function getOffset(
    DateTimeInterface $datetime,
  )[]: HH\FIXME\MISSING_RETURN_TYPE;
  public function getTransitions(
    int $timestamp_begin = PHP_INT_MIN,
    int $timestamp_end = PHP_INT_MAX,
  )[]: HH\FIXME\MISSING_RETURN_TYPE;
  static public function listAbbreviations()[]: HH\FIXME\MISSING_RETURN_TYPE;
  static public function listIdentifiers(
    int $what = 2047,
    string $country = '',
  )[]: HH\FIXME\MISSING_RETURN_TYPE;
}

class DateInterval {
  public int $y;
  public int $m;
  public int $d;
  public int $h;
  public int $i;
  public int $s;
  public int $invert;
  public mixed $days;

  public function __construct(string $interval_spec);
  static public function createFromDateString(
    string $time,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function format(string $format): HH\FIXME\MISSING_RETURN_TYPE;
}

class DatePeriod implements Iterator<DateTime> {
  const int EXCLUDE_START_DATE;

  public function __construct(
    HH\FIXME\MISSING_PARAM_TYPE /* DateTimeInterface */
      $start, // date string converts
    DateInterval $interval,
    HH\FIXME\MISSING_PARAM_TYPE /* ?DateTimeInterface */ $end =
      null, // date string converts
    int $options = 0,
  );
  public function current(): DateTime;
  public function rewind(): void;
  public function key(): HH\FIXME\MISSING_RETURN_TYPE;
  public function next(): void;
  public function valid(): bool;
  public function getStartDate(): DateTime;
  public function getEndDate(): DateTime;
  public function getDateInterval(): DateInterval;
}
