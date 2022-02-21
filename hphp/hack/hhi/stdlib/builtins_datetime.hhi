<?hh    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const string DATE_ATOM    = "";
const string DATE_COOKIE  = "";
const string DATE_ISO8601 = "";
const string DATE_RFC1036 = "";
const string DATE_RFC1123 = "";
const string DATE_RFC2822 = "";
const string DATE_RFC3339 = "";
const string DATE_RFC822  = "";
const string DATE_RFC850  = "";
const string DATE_RSS     = "";
const string DATE_W3C     = "";

const int DAY_1 = 131079;
const int DAY_2 = 131080;
const int DAY_3 = 131081;
const int DAY_4 = 131082;
const int DAY_5 = 131083;
const int DAY_6 = 131084;
const int DAY_7 = 131085;

type DateTimeErrors = shape(
  'warning_count' => int,
  'warnings' => darray<int, string>,
  'error_count' => int,
  'errors' => darray<int, string>
);

<<__PHPStdLib>>
function checkdate(int $month, int $day, int $year);
<<__PHPStdLib>>
function date_add($datetime, $interval);
<<__PHPStdLib>>
function date_create_from_format(string $format, string $time, $timezone = null);
<<__PHPStdLib>>
function date_create($time = null, $timezone = null);
<<__PHPStdLib>>
function date_date_set($object, int $year, int $month, int $day);
<<__PHPStdLib>>
function date_default_timezone_get();
<<__PHPStdLib>>
function date_default_timezone_set(string $name);
<<__PHPStdLib>>
function date_diff($datetime, $datetime2, bool $absolute = false);
<<__PHPStdLib>>
function date_format($object, string $format);
<<__PHPStdLib>>
function date_get_last_errors();
<<__PHPStdLib>>
function date_interval_create_from_date_string(string $time);
<<__PHPStdLib>>
function date_interval_format($interval, string $format_spec);
<<__PHPStdLib>>
function date_isodate_set($object, int $year, int $week, int $day = 1);
<<__PHPStdLib>>
function date_modify($object, string $modify);
<<__PHPStdLib>>
function date_offset_get($object);
<<__PHPStdLib>>
function date_parse(string $date);
<<__PHPStdLib>>
function date_sub($datetime, $interval);
<<__PHPStdLib>>
function date_sun_info(int $ts, float $latitude, float $longitude);
<<__PHPStdLib>>
function date_sunrise(int $timestamp, int $format = 0, ?float $latitude = null, ?float $longitude = null, ?float $zenith = null, ?float $gmt_offset = null);
<<__PHPStdLib>>
function date_sunset(int $timestamp, int $format = 0, ?float $latitude = null, ?float $longitude = null, ?float $zenith = null, ?float $gmt_offset = null);
<<__PHPStdLib>>
function date_time_set($object, int $hour, int $minute, int $second = 0);
<<__PHPStdLib>>
function date_timestamp_get($datetime);
<<__PHPStdLib>>
function date_timestamp_set($datetime, int $timestamp);
<<__PHPStdLib>>
function date_timezone_get($object);
<<__PHPStdLib>>
function date_timezone_set($object, $timezone);
<<__PHPStdLib>>
function date(string $format, ?int $timestamp = null)/*: string*/ { }
<<__PHPStdLib>>
function getdate(?int $timestamp = null);
<<__PHPStdLib>>
function gettimeofday(bool $return_float = false);
<<__PHPStdLib>>
function gmdate(string $format, ?int $timestamp = null);
<<__PHPStdLib>>
function gmmktime(int $hour = PHP_INT_MAX, int $minute = PHP_INT_MAX, int $second = PHP_INT_MAX, int $month = PHP_INT_MAX, int $day = PHP_INT_MAX, int $year = PHP_INT_MAX);
<<__PHPStdLib>>
function gmstrftime(string $format, ?int $timestamp = null);
<<__PHPStdLib>>
function idate(string $format, ?int $timestamp = null);
<<__PHPStdLib>>
function localtime(?int $timestamp = null, bool $is_associative = false);
<<__PHPStdLib>>
function microtime(bool $get_as_float = false)[leak_safe];
<<__PHPStdLib>>
function mktime(int $hour = PHP_INT_MAX, int $minute = PHP_INT_MAX, int $second = PHP_INT_MAX, int $month = PHP_INT_MAX, int $day = PHP_INT_MAX, int $year = PHP_INT_MAX)[leak_safe];
<<__PHPStdLib>>
function strftime(string $format, ?int $timestamp = null)[leak_safe];
<<__PHPStdLib>>
function strptime(string $date, string $format);
<<__PHPStdLib>>
function strtotime(string $input, ?int $timestamp = null)[leak_safe];
<<__PHPStdLib>>
function time()[leak_safe]: int;
<<__PHPStdLib>>
function timezone_abbreviations_list();
<<__PHPStdLib>>
function timezone_identifiers_list(int $what = 2047, string $country = '');
<<__PHPStdLib>>
function timezone_location_get($timezone);
<<__PHPStdLib>>
function timezone_name_from_abbr(string $abbr, int $gmtoffset = -1, int $isdst = 1);
<<__PHPStdLib>>
function timezone_name_get($object);
<<__PHPStdLib>>
function timezone_offset_get($object, $dt);
<<__PHPStdLib>>
function timezone_open(string $timezone);
<<__PHPStdLib>>
function timezone_transitions_get(DateTimeZone $object,
                                  int $timestamp_begin = PHP_INT_MIN,
                                  int $timestamp_end = PHP_INT_MAX);
<<__PHPStdLib>>
function timezone_version_get();

interface DateTimeInterface {
  public function diff(DateTimeInterface $datetime2, bool $absolute = false);
  public function format(string $format);
  public function getOffset();
  public function getTimestamp()[];
  public function getTimezone();
}

class DateTime implements DateTimeInterface {
  const string ATOM = '';
  const string COOKIE = '';
  const string ISO8601 = '';
  const string RFC822 = '';
  const string RFC850 = '';
  const string RFC1036 = '';
  const string RFC1123 = '';
  const string RFC2822 = '';
  const string RFC3339 = '';
  const string RSS = '';
  const string W3C = '';

  public function __construct(string $time = 'now', ?DateTimeZone $timezone = null)[leak_safe];
  public function add(DateInterval $interval);
  public function modify(string $modify);
  public function getOffset(): int;
  public function getTimestamp()[]: int;
  public function getTimezone();
  public function setDate(int $year, int $month, int $day);
  public function setISODate(int $year, int $week, int $day = 1);
  public function setTime(int $hour, int $minute, int $second = 0);
  public function setTimestamp(int $unixtimestamp): this;
  public function setTimezone(DateTimeZone $timezone);
  public function sub(DateInterval $interval);
  public function diff(DateTimeInterface $datetime2, bool $absolute = false);
  public function format(string $format);
  public static function createFromFormat(
    string $format,
    string $time,
    ?DateTimeZone $timezone = null,
  );
  public static function getLastErrors(): DateTimeErrors;
}


class DateTimeImmutable implements DateTimeInterface {
  private DateTime $data;

  public function __construct(string $time = 'now', ?DateTimeZone $timezone = null);
  public function add(DateInterval $interval): this;
  public function modify(string $modify): this;
  public function getOffset(): int;
  public function getTimestamp()[]: int;
  public function getTimezone();
  public function setDate(int $year, int $month, int $day): this;
  public function setISODate(int $year, int $week, int $day = 1): this;
  public function setTime(int $hour, int $minute, int $second = 0): this;
  public function setTimestamp(int $unixtimestamp): this;
  public function setTimezone(DateTimeZone $timezone): this;
  public function sub(DateInterval $interval): this;
  public function diff(DateTimeInterface $datetime2, bool $absolute = false);
  public function format(string $format);
  public static function createFromFormat(
    string $format,
    string $time,
    ?DateTimeZone $timezone = null,
  );
  public static function createFromMutable(DateTime $datetime);
  public static function getLastErrors(): DateTimeErrors;
  public function __clone();
}

class DateTimeZone {
  const int AFRICA = 0;
  const int AMERICA = 0;
  const int ANTARCTICA = 0;
  const int ARCTIC = 0;
  const int ASIA = 0;
  const int ATLANTIC = 0;
  const int AUSTRALIA = 0;
  const int EUROPE = 0;
  const int INDIAN = 0;
  const int PACIFIC = 0;
  const int UTC = 0;
  const int ALL = 0;
  const int ALL_WITH_BC = 0;
  const int PER_COUNTRY = 0;
  public function __construct(string $timezone)[];
  public function getLocation()[]: darray { }
  public function getName()[]: string { }
  public function getOffset(DateTimeInterface $datetime)[];
  public function getTransitions(int $timestamp_begin = PHP_INT_MIN,
                                 int $timestamp_end = PHP_INT_MAX)[];
  static public function listAbbreviations()[];
  static public function listIdentifiers(int $what = 2047, string $country = '')[];
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
  static public function createFromDateString(string $time);
  public function format(string $format);
}

class DatePeriod implements Iterator<DateTime> {
  const int EXCLUDE_START_DATE = 1;

  public function __construct(
    /* DateTimeInterface */ $start, // date string converts
    DateInterval $interval,
    /* ?DateTimeInterface */ $end = null, // date string converts
    int $options = 0,
  );
  public function current(): DateTime;
  public function rewind(): void;
  public function key();
  public function next();
  public function valid(): bool;
  public function getStartDate(): DateTime;
  public function getEndDate(): DateTime;
  public function getDateInterval(): DateInterval;
}
