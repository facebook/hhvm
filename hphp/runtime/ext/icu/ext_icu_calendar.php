<?hh // partial

<<__NativeData>>
class IntlCalendar {
  /**
   * Add a (signed) amount of time to a field
   *
   * @param int $field -
   * @param int $amount -
   *
   * @return bool - Returns TRUE on success.
   */
  <<__Native>>
  public function add(int $field,
                      int $amount): bool;

  /**
   * Whether this object's time is after that of the passed object
   *
   * @param IntlCalendar $other -
   *
   * @return bool - Returns TRUE if this object's current time is after
   *   that of the calendar argument's time. Returns FALSE otherwise. Also
   *   returns FALSE on failure. You can use exceptions or
   *   intl_get_error_code() to detect error conditions.
   */
  <<__Native>>
  public function after(IntlCalendar $other): bool;

  /**
   * Whether this object's time is before that of the passed object
   *
   * @param IntlCalendar $other -
   *
   * @return bool - Returns TRUE if this object's current time is before
   *   that of the calendar argument's time. Returns FALSE otherwise. Also
   *   returns FALSE on failure. You can use exceptions or
   *   intl_get_error_code() to detect error conditions.
   */
  <<__Native>>
  public function before(IntlCalendar $other): bool;

  /**
   * Clear a field or all fields
   *
   * @param int $field -
   *
   * @return bool - Returns TRUE on success. Failure can only occur is
   *   invalid arguments are provided.
   */
  <<__Native>>
  public function clear(?int $field = NULL): bool;

  /**
   * Private constructor for disallowing instantiation
   *
   * @return  -
   */
  private function __construct(): void {}

  /**
   * Create a new IntlCalendar
   *
   * @param mixed $timeZone -
   * @param string $locale -
   *
   * @return IntlCalendar - The created IntlCalendar instance or NULL on
   *   failure.
   */
  <<__Native>>
  public static function createInstance(mixed $timeZone = NULL,
                                        string $locale = ""): IntlCalendar;

  /**
   * Compare time of two IntlCalendar objects for equality
   *
   * @param IntlCalendar $other -
   *
   * @return bool - Returns TRUE if the current time of both this and the
   *   passed in IntlCalendar object are the same, or FALSE otherwise. The
   *   value FALSE can also be returned on failure. This can only happen if
   *   bad arguments are passed in. In any case, the two cases can be
   *   distinguished by calling intl_get_error_code().
   */
  <<__Native>>
  public function equals(IntlCalendar $other): bool;

  /**
   * Calculate difference between given time and this object's time
   *
   * @param float $when -
   * @param int $field -
   *
   * @return int - Returns a (signed) difference of time in the unit
   *   associated with the specified field.
   */
  <<__Native>>
  public function fieldDifference(mixed $when,
                                  int $field): mixed;

  /**
   * Create an IntlCalendar from a DateTime object or string
   *
   * @param mixed $dateTime -
   *
   * @return IntlCalendar - The created IntlCalendar object or NULL in
   *   case of failure. If a string is passed, any exception that occurs
   *   inside the DateTime constructor is propagated.
   */
  public static function fromDateTime(mixed $dateTime,
                                      string $locale = ""): IntlCalendar {
    if (!($dateTime is DateTime)) {
      $dateTime = new DateTime($dateTime);
    }
    if (!($locale ?? false)) {
      $locale = ini_get("intl.default_locale");
    }
    $cal = IntlCalendar::createInstance($dateTime->getTimezone(), $locale);
    $cal->setTime($dateTime->getTimeStamp() * 1000);
    return $cal;
  }

  /**
   * Get the value for a field
   *
   * @param int $field -
   *
   * @return int - An integer with the value of the time field.
   */
  <<__Native>>
  public function get(int $field): mixed;

  /**
   * The maximum value for a field, considering the object's current time
   *
   * @param int $field -
   *
   * @return int - An int representing the maximum value in the units
   *   associated with the given field.
   */
  <<__Native>>
  public function getActualMaximum(int $field): mixed;

  /**
   * The minimum value for a field, considering the object's current time
   *
   * @param int $field -
   *
   * @return int - An int representing the minimum value in the field's
   *   unit.
   */
  <<__Native>>
  public function getActualMinimum(int $field): mixed;

  /**
   * Get array of locales for which there is data
   *
   * @return array - An array of strings, one for which locale.
   */
  <<__Native>>
  public static function getAvailableLocales(): varray;

  /**
   * Tell whether a day is a weekday, weekend or a day that has a transition
   * between the two
   *
   * @param int $dayOfWeek -
   *
   * @return int - Returns one of the constants
   *   IntlCalendar::DOW_TYPE_WEEKDAY, IntlCalendar::DOW_TYPE_WEEKEND,
   *   IntlCalendar::DOW_TYPE_WEEKEND_OFFSET or
   *   IntlCalendar::DOW_TYPE_WEEKEND_CEASE.
   */
  <<__Native>>
  public function getDayOfWeekType(int $dayOfWeek): mixed;

  /**
   * Get last error code on the object
   *
   * @return int - An ICU error code indicating either success, failure
   *   or a warning.
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get last error message on the object
   *
   * @return string - The error message associated with last error that
   *   occurred in a function call on this object, or a string indicating
   *   the non-existence of an error.
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Get the first day of the week for the calendar's locale
   *
   * @return int - One of the constants IntlCalendar::DOW_SUNDAY,
   *   IntlCalendar::DOW_MONDAY, ..., IntlCalendar::DOW_SATURDAY.
   */
  <<__Native>>
  public function getFirstDayOfWeek(): mixed;

  /**
   * Get the largest local minimum value for a field
   *
   * @param int $field -
   *
   * @return int - An int representing a field value, in the field's
   *   unit,.
   */
  <<__Native>>
  public function getGreatestMinimum(int $field): mixed;

  /**
   * Get set of locale keyword values
   *
   * @param string $key -
   * @param string $locale -
   * @param boolean $commonlyUsed -
   *
   * @return Iterator - An iterator that yields strings with the locale
   *   keyword values.
   */
  <<__Native>>
  public static function getKeywordValuesForLocale(string $key,
                                                   string $locale,
                                                   bool $commonlyUsed): mixed;

  /**
   * Get the smallest local maximum for a field
   *
   * @param int $field -
   *
   * @return int - An int representing a field value in the field's
   *   unit.
   */
  <<__Native>>
  public function getLeastMaximum(int $field): mixed;

  /**
   * Get the locale associated with the object
   *
   * @param int $localeType -
   *
   * @return string - A locale string.
   */
  <<__Native>>
  public function getLocale(int $localeType): mixed;

  /**
   * Get the global maximum value for a field
   *
   * @param int $field -
   *
   * @return int - An int representing a field value in the field's
   *   unit.
   */
  <<__Native>>
  public function getMaximum(int $field): mixed;

  /**
   * Get minimal number of days the first week in a year or month can have
   *
   * @return int - An int representing a number of days.
   */
  <<__Native>>
  public function getMinimalDaysInFirstWeek(): mixed;

  /**
   * Get the global minimum value for a field
   *
   * @param int $field -
   *
   * @return int - An int representing a value for the given field in the
   *   field's unit.
   */
  <<__Native>>
  public function getMinimum(int $field): mixed;

  /**
   * Get number representing the current time
   *
   * @return float - A float representing a number of milliseconds since
   *   the epoch, not counting leap seconds.
   */
  <<__Native>>
  public static function getNow(): float;

  /**
   * Get behavior for handling repeating wall time
   *
   * @return int - One of the constants IntlCalendar::WALLTIME_FIRST or
   *   IntlCalendar::WALLTIME_LAST.
   */
  <<__Native>>
  public function getRepeatedWallTimeOption(): int;

  /**
   * Get behavior for handling skipped wall time
   *
   * @return int - One of the constants IntlCalendar::WALLTIME_FIRST,
   *   IntlCalendar::WALLTIME_LAST or IntlCalendar::WALLTIME_NEXT_VALID.
   */
  <<__Native>>
  public function getSkippedWallTimeOption(): int;

  /**
   * Get time currently represented by the object
   *
   * @return float - A float representing the number of milliseconds
   *   elapsed since the reference time (1 Jan 1970 00:00:00 UTC).
   */
  <<__Native>>
  public function getTime(): mixed;

  /**
   * Get the object's timezone
   *
   * @return IntlTimeZone - An IntlTimeZone object corresponding to the
   *   one used internally in this object.
   */
  <<__Native>>
  public function getTimezone(): IntlTimeZone;

  /**
   * Get the calendar type
   *
   * @return string - A string representing the calendar type, such as
   *   'gregorian', 'islamic', etc.
   */
  <<__Native>>
  public function getType(): mixed;

  /**
   * Get time of the day at which weekend begins or ends
   *
   * @param string $dayOfWeek -
   *
   * @return int - The number of milliseconds into the day at which the
   *   the weekend begins or ends.
   */
  <<__Native>>
  public function getWeekendTransition(int $dayOfWeek): mixed;

  /**
   * Whether the object's time is in Daylight Savings Time
   *
   * @return bool - Returns TRUE if the date is in Daylight Savings Time,
   *   FALSE otherwise. The value FALSE may also be returned on failure,
   *   for instance after specifying invalid field values on non-lenient
   *   mode; use exceptions or query intl_get_error_code() to disambiguate.
   */
  <<__Native>>
  public function inDaylightTime(): bool;

  /**
   * Whether another calendar is equal but for a different time
   *
   * @param IntlCalendar $other -
   *
   * @return bool - Assuming there are no argument errors, returns TRUE
   *   iif the calendars are equivalent except possibly for their set time.
   */
  <<__Native>>
  public function isEquivalentTo(IntlCalendar $other): bool;

  /**
   * Whether date/time interpretation is in lenient mode
   *
   * @return bool - A bool representing whether the calendar is set to
   *   lenient mode.
   */
  <<__Native>>
  public function isLenient(): bool;

  /**
   * Whether a field is set
   *
   * @param int $field -
   *
   * @return bool - Assuming there are no argument errors, returns TRUE
   *   iif the field is set.
   */
  <<__Native>>
  public function isSet(int $field): bool;

  /**
   * Whether a certain date/time is in the weekend
   *
   * @param float $date -
   *
   * @return bool - A bool indicating whether the given or this object's
   *   time occurs in a weekend.   The value FALSE may also be returned on
   *   failure, for instance after giving a date out of bounds on
   *   non-lenient mode; use exceptions or query intl_get_error_code() to
   *   disambiguate.
   */
  <<__Native>>
  public function isWeekend(mixed $date = NULL): bool;

  /**
   * Add value to field without carrying into more significant fields
   *
   * @param int $field -
   * @param mixed $amountOrUpOrDown -
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function roll(int $field,
                       mixed $amountOrUpOrDown): bool;

  /**
   * Set a time field or several common fields at once
   *
   * Variant 1:
   * @param int $field
   * @param int $value
   *
   * Variant 2:
   * @param int $year -
   * @param int $month -
   * @param int $dayOfMonth -
   * @param int $hour -
   * @param int $minute -
   * @param int $second -
   *
   * @return bool - Returns TRUE on success and FALSE on failure.
   */
  <<__Native>>
  public function set(int $yearOrField,
                      int $monthOrValue,
                      ?int $dayOfMonth = NULL,
                      ?int $hour = NULL,
                      ?int $minute = NULL,
                      ?int $second = NULL): bool;

  /**
   * Set the day on which the week is deemed to start
   *
   * @param int $dayOfWeek -
   *
   * @return bool - Returns TRUE on success. Failure can only happen due
   *   to invalid parameters.
   */
  <<__Native>>
  public function setFirstDayOfWeek(int $dayOfWeek): bool;

  /**
   * Set whether date/time interpretation is to be lenient
   *
   * @param bool $isLenient -
   *
   * @return bool - Returns TRUE on success. Failure can only
   *   happen due to invalid parameters.
   */
  <<__Native>>
  public function setLenient(bool $isLenient): bool;

  /**
   * Set minimal number of days the first week in a year or month can have
   *
   * @param int $minimalDays -
   *
   * @return bool - TRUE on success, FALSE on failure.
   */
  <<__Native>>
  public function setMinimalDaysInFirstWeek(int $minimalDays): bool;

  /**
   * Set behavior for handling repeating wall times at negative timezone
   * offset transitions
   *
   * @param int $wallTimeOption -
   *
   * @return bool - Returns TRUE on success. Failure can only happen due
   *   to invalid parameters.
   */
  <<__Native>>
  public function setRepeatedWallTimeOption(int $wallTimeOption): bool;

  /**
   * Set behavior for handling skipped wall times at positive timezone offset
   * transitions
   *
   * @param int $wallTimeOption -
   *
   * @return bool - Returns TRUE on success. Failure can only happen due
   *   to invalid parameters.
   */
  <<__Native>>
  public function setSkippedWallTimeOption(int $wallTimeOption): bool;

  /**
   * Set the calendar time in milliseconds since the epoch
   *
   * @param float $date -
   *
   * @return bool - Returns TRUE on success and FALSE on failure.
   */
  <<__Native>>
  public function setTime(mixed $date): bool;

  /**
   * Set the timezone used by this calendar
   *
   * @param mixed $timeZone -
   *
   * @return bool - Returns TRUE on success and FALSE on failure.
   */
  <<__Native>>
  public function setTimezone(mixed $timeZone): bool;

  /**
   * Convert an IntlCalendar into a DateTime object
   *
   * @return DateTime - A DateTime object with the same timezone as this
   *   object (though using PHP's database instead of ICU's) and the same
   *   time, except for the smaller precision (second precision instead of
   *   millisecond). Returns FALSE on failure.
   */
  public function toDateTime(): DateTime {
    $dtz = $this->getTimezone()->toDateTimeZone();
    $dt  = new DateTime('@'.(int)($this->getTime()/1000), $dtz);
    $dt->setTimezone($dtz);
    return $dt;
  }
}

/**
 * Add a (signed) amount of time to a field
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 * @param int $amount -
 *
 * @return bool - Returns TRUE on success.
 */
function intlcal_add(IntlCalendar $cal,
                     int $field,
                     int $amount): bool {
  return $cal->add($field, $amount);
}

/**
 * Whether this object's time is after that of the passed object
 *
 * @param IntlCalendar $cal -
 * @param IntlCalendar $other -
 *
 * @return bool - Returns TRUE if this object's current time is after
 *   that of the calendar argument's time. Returns FALSE otherwise. Also
 *   returns FALSE on failure. You can use exceptions or
 *   intl_get_error_code() to detect error conditions.
 */
function intlcal_after(IntlCalendar $cal,
                       IntlCalendar $other): bool {
  return $cal->after($other);
}

/**
 * Whether this object's time is before that of the passed object
 *
 * @param IntlCalendar $cal -
 * @param IntlCalendar $other -
 *
 * @return bool - Returns TRUE if this object's current time is before
 *   that of the calendar argument's time. Returns FALSE otherwise. Also
 *   returns FALSE on failure. You can use exceptions or
 *   intl_get_error_code() to detect error conditions.
 */
function intlcal_before(IntlCalendar $cal,
                IntlCalendar $other): bool {
  return $cal->before($other);
}

/**
 * Clear a field or all fields
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return bool - Returns TRUE on success. Failure can only occur is
 *   invalid arguments are provided.
 */
function intlcal_clear(IntlCalendar $cal,
                      ?int $field = NULL): bool {
  return $cal->clear($field);
}

/**
 * Create a new IntlCalendar
 *
 * @param mixed $timeZone -
 * @param string $locale -
 *
 * @return IntlCalendar - The created IntlCalendar instance or NULL on
 *   failure.
 */
function intlcal_create_instance(mixed $timeZone = NULL,
                                 string $locale = ""): IntlCalendar {
  return IntlCalendar::createInstance($timeZone, $locale);
}

/**
 * Compare time of two IntlCalendar objects for equality
 *
 * @param IntlCalendar $cal -
 * @param IntlCalendar $other -
 *
 * @return bool - Returns TRUE if the current time of both this and the
 *   passed in IntlCalendar object are the same, or FALSE otherwise. The
 *   value FALSE can also be returned on failure. This can only happen if
 *   bad arguments are passed in. In any case, the two cases can be
 *   distinguished by calling intl_get_error_code().
 */
function intlcal_equals(IntlCalendar $cal,
                        IntlCalendar $other): bool {
  return $cal->equals($other);
}

/**
 * Calculate difference between given time and this object's time
 *
 * @param IntlCalendar $cal -
 * @param float $when -
 * @param int $field -
 *
 * @return int - Returns a (signed) difference of time in the unit
 *   associated with the specified field.
 */
function intlcal_field_difference(IntlCalendar $cal,
                                  mixed $when,
                                  int $field): mixed {
  return $cal->fieldDifference($when, $field);
}

/**
 * Create an IntlCalendar from a DateTime object or string
 *
 * @param mixed $dateTime -
 *
 * @return IntlCalendar - The created IntlCalendar object or NULL in case
 *   of failure. If a string is passed, any exception that occurs inside
 *   the DateTime constructor is propagated.
 */
function intlcal_from_date_time(mixed $dateTime,
                                string $locale = ""): IntlCalendar {
  return IntlCalendar::fromDateTime($dateTime, $locale);
}

/**
 * Get the value for a field
 *
 * @param IntlCalendar> $cal -
 * @param int $field -
 *
 * @return int - An integer with the value of the time field.
 */
function intlcal_get(IntlCalendar $cal,
                     int $field): mixed {
  return $cal->get($field);
}

/**
 * The maximum value for a field, considering the object's current time
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return int - An int representing the maximum value in the units
 *   associated with the given field.
 */
function intlcal_get_actual_maximum(IntlCalendar $cal,
                                   int $field): mixed {
  return $cal->getActualMaximum($field);
}

/**
 * The minimum value for a field, considering the object's current time
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return int - An int representing the minimum value in the field's
 *   unit.
 */
function intlcal_get_actual_minimum(IntlCalendar $cal,
                                    int $field): mixed {
  return $cal->getActualMinimum($field);
}

/**
 * Get array of locales for which there is data
 *
 * @return array - An array of strings, one for which locale.
 */
function intlcal_get_available_locales(): varray {
  return IntlCalendar::getAvailableLocales();
}

/**
 * Tell whether a day is a weekday, weekend or a day that has a transition
 * between the two
 *
 * @param IntlCalendar $cal -
 * @param int $dayOfWeek -
 *
 * @return int - Returns one of the constants
 *   IntlCalendar::DOW_TYPE_WEEKDAY, IntlCalendar::DOW_TYPE_WEEKEND,
 *   IntlCalendar::DOW_TYPE_WEEKEND_OFFSET or
 *   IntlCalendar::DOW_TYPE_WEEKEND_CEASE.
 */
function intlcal_get_day_of_week_type(IntlCalendar $cal,
                                      int $dayOfWeek): mixed {
  return $cal->getDayOfWeekType($dayOfWeek);
}

/**
 * Get last error code on the object
 *
 * @param IntlCalendar $calendar -
 *
 * @return int - An ICU error code indicating either success, failure or
 *   a warning.
 */
function intlcal_get_error_code(IntlCalendar $cal): int {
  return $cal->getErrorCode();
}

/**
 * Get last error message on the object
 *
 * @param IntlCalendar $calendar -
 *
 * @return string - The error message associated with last error that
 *   occurred in a function call on this object, or a string indicating the
 *   non-existence of an error.
 */
function intlcal_get_error_message(IntlCalendar $cal): string {
  return $cal->getErrorMessage();
}

/**
 * Get the first day of the week for the calendar's locale
 *
 * @param IntlCalendar $cal -
 *
 * @return int - One of the constants IntlCalendar::DOW_SUNDAY,
 *   IntlCalendar::DOW_MONDAY, ..., IntlCalendar::DOW_SATURDAY.
 */
function intlcal_get_first_day_of_week(IntlCalendar $cal): mixed {
  return $cal->getFirstDayOfWeek();
}

/**
 * Get the largest local minimum value for a field
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return int - An int representing a field value, in the field's
 *   unit,.
 */
function intlcal_get_greatest_minimum(IntlCalendar $cal,
                                      int $field): mixed {
  return $cal->getGreatestMinimum($field);
}

/**
 * Get set of locale keyword values
 *
 * @param string $key -
 * @param string $locale -
 * @param boolean $commonlyUsed -
 *
 * @return Iterator - An iterator that yields strings with the locale
 *   keyword values.
 */
function intlcal_get_keyword_values_for_locale(string $key,
                                               string $locale,
                                               bool $commonlyUsed): mixed {
  return IntlCalendar::getKeywordValuesForLocale($key, $locale, $commonlyUsed);
}

/**
 * Get the smallest local maximum for a field
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return int - An int representing a field value in the field's unit.
 */
function intlcal_get_least_maximum(IntlCalendar $cal,
                                   int $field): mixed {
  return $cal->getLeastMaximum($field);
}

/**
 * Get the locale associated with the object
 *
 * @param IntlCalendar $cal -
 * @param int $localeType -
 *
 * @return string - A locale string.
 */
function intlcal_get_locale(IntlCalendar $cal,
                            int $localeType): mixed {
  return $cal->getLocale($localeType);
}

/**
 * Get the global maximum value for a field
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return int - An int representing a field value in the field's unit.
 */
function intlcal_get_maximum(IntlCalendar $cal,
                             int $field): mixed {
  return $cal->getMaximum($field);
}

/**
 * Set minimal number of days the first week in a year or month can have
 *
 * @param IntlCalendar $cal -
 *
 * @return bool - TRUE on success, FALSE on failure.
 */
function intlcal_get_minimal_days_in_first_week(IntlCalendar $cal): mixed {
  return $cal->getMinimalDaysInFirstWeek();
}

/**
 * Get the global minimum value for a field
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return int - An int representing a value for the given field in the
 *   field's unit.
 */
function intlcal_get_minimum(IntlCalendar $cal,
                             int $field): mixed {
  return $cal->getMinimum($field);
}

/**
 * Get number representing the current time
 *
 * @return float - A float representing a number of milliseconds since
 *   the epoch, not counting leap seconds.
 */
function intlcal_get_now(): float {
  return IntlCalendar::getNow();
}

/**
 * Get behavior for handling repeating wall time
 *
 * @param IntlCalendar $cal -
 *
 * @return int - One of the constants IntlCalendar::WALLTIME_FIRST or
 *   IntlCalendar::WALLTIME_LAST.
 */
function intlcal_get_repeated_wall_time_option(IntlCalendar $cal): int {
  return $cal->getRepeatedWallTimeOption();
}

/**
 * Get behavior for handling skipped wall time
 *
 * @param IntlCalendar $cal -
 *
 * @return int - One of the constants IntlCalendar::WALLTIME_FIRST,
 *   IntlCalendar::WALLTIME_LAST or IntlCalendar::WALLTIME_NEXT_VALID.
 */
function intlcal_get_skipped_wall_time_option(IntlCalendar $cal): int {
  return $cal->getSkippedWallTimeOption();
}

/**
 * Get time currently represented by the object
 *
 * @param IntlCalendar $cal -
 *
 * @return float - A float representing the number of milliseconds
 *   elapsed since the reference time (1 Jan 1970 00:00:00 UTC).
 */
function intlcal_get_time(IntlCalendar $cal): mixed {
  return $cal->getTime();
}

/**
 * Get the object's timezone
 *
 * @param IntlCalendar $cal -
 *
 * @return IntlTimeZone - An IntlTimeZone object corresponding to the one
 *   used internally in this object.
 */
function intlcal_get_time_zone(IntlCalendar $cal): IntlTimeZone {
  return $cal->getTimezone();
}

/**
 * Get the calendar type
 *
 * @param IntlCalendar $cal -
 *
 * @return string - A string representing the calendar type, such as
 *   'gregorian', 'islamic', etc.
 */
function intlcal_get_type(IntlCalendar $cal): mixed {
  return $cal->getType();
}

/**
 * Get time of the day at which weekend begins or ends
 *
 * @param IntlCalendar $cal -
 * @param string $dayOfWeek -
 *
 * @return int - The number of milliseconds into the day at which the the
 *   weekend begins or ends.
 */
function intlcal_get_weekend_transition(IntlCalendar $cal,
                                        int $dayOfWeek): mixed {
  return $cal->getWeekendTransition($dayOfWeek);
}

/**
 * Whether the object's time is in Daylight Savings Time
 *
 * @param IntlCalendar $cal -
 *
 * @return bool - Returns TRUE if the date is in Daylight Savings Time,
 *   FALSE otherwise. The value FALSE may also be returned on failure, for
 *   instance after specifying invalid field values on non-lenient mode;
 *   use exceptions or query intl_get_error_code() to disambiguate.
 */
function intlcal_in_daylight_time(IntlCalendar $cal): bool {
  return $cal->inDaylightTime();
}

/**
 * Whether another calendar is equal but for a different time
 *
 * @param IntlCalendar $cal -
 * @param IntlCalendar $other -
 *
 * @return bool - Assuming there are no argument errors, returns TRUE iif
 *   the calendars are equivalent except possibly for their set time.
 */
function intlcal_is_equivalent_to(IntlCalendar $cal,
                                  IntlCalendar $other): bool {
  return $cal->isEquivalentTo($other);
}

/**
 * Whether date/time interpretation is in lenient mode
 *
 * @param IntlCalendar $cal -
 *
 * @return bool - A bool representing whether the calendar is set to
 *   lenient mode.
 */
function intlcal_is_lenient(IntlCalendar $cal): bool {
  return $cal->isLenient();
}

/**
 * Whether a field is set
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 *
 * @return bool - Assuming there are no argument errors, returns TRUE iif
 *   the field is set.
 */
function intlcal_is_set(IntlCalendar $cal,
                        int $field): bool {
  return $cal->isSet($field);
}

/**
 * Whether a certain date/time is in the weekend
 *
 * @param IntlCalendar $cal -
 * @param float $date -
 *
 * @return bool - A bool indicating whether the given or this object's
 *   time occurs in a weekend.   The value FALSE may also be returned on
 *   failure, for instance after giving a date out of bounds on non-lenient
 *   mode; use exceptions or query intl_get_error_code() to disambiguate.
 */
function intlcal_is_weekend(IntlCalendar $cal,
                            mixed $date = NULL): bool {
  return $cal->isWeekend($date);
}

/**
 * Add value to field without carrying into more significant fields
 *
 * @param IntlCalendar $cal -
 * @param int $field -
 * @param mixed $amountOrUpOrDown -
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
function intlcal_roll(IntlCalendar $cal,
                      int $field,
                      mixed $amountOrUpOrDown): bool {
  return $cal->roll($field, $amountOrUpOrDown);
}

/**
 * Set a time field or several common fields at once
 *
 * @param IntlCalendar $cal -
 * @param int $year -
 * @param int $month -
 * @param int $dayOfMonth -
 * @param int $hour -
 * @param int $minute -
 * @param int $second -
 *
 * @return bool - Returns TRUE on success and FALSE on failure.
 */
function intlcal_set(IntlCalendar $cal,
                     int $year,
                     int $month,
                     ?int $dayOfMonth = NULL,
                     ?int $hour = NULL,
                     ?int $minute = NULL,
                     ?int $second = NULL): bool {
  return $cal->set($year, $month, $dayOfMonth,
                   $hour, $minute, $second);
}

/**
 * Set the day on which the week is deemed to start
 *
 * @param IntlCalendar $cal -
 * @param int $dayOfWeek -
 *
 * @return bool - Returns TRUE on success. Failure can only happen due to
 *   invalid parameters.
 */
function intlcal_set_first_day_of_week(IntlCalendar $cal,
                                       int $dayOfWeek): bool {
  return $cal->setFirstDayOfWeek($dayOfWeek);
}

/**
 * Set whether date/time interpretation is to be lenient
 *
 * @param IntlCalendar $cal -
 * @param bool $isLenient -
 *
 * @return bool - Returns TRUE on success. Failure can only happen
 *   due to invalid parameters.
 */
function intlcal_set_lenient(IntlCalendar $cal,
                             bool $isLenient): bool {
  return $cal->setLenient($isLenient);
}

/**
 * Set minimal number of days the first week in a year or month can have
 *
 * @param int $minimalDays -
 *
 * @return bool - TRUE on success, FALSE on failure.
 */
function intlcal_set_minimal_days_in_first_week(IntlCalendar $cal,
                                                int $minimalDays): bool {
  return $cal->setMinimalDaysInFirstWeek($minimalDays);
}

/**
 * Set behavior for handling repeating wall times at negative timezone offset
 * transitions
 *
 * @param IntlCalendar $cal -
 * @param int $wallTimeOption -
 *
 * @return bool - Returns TRUE on success. Failure can only happen due to
 *   invalid parameters.
 */
function intlcal_set_repeated_wall_time_option(IntlCalendar $cal,
                                               int $wallTimeOption): bool {
  return $cal->setRepeatedWallTimeOption($wallTimeOption);
}

/**
 * Set behavior for handling skipped wall times at positive timezone offset
 * transitions
 *
 * @param IntlCalendar $cal -
 * @param int $wallTimeOption -
 *
 * @return bool - Returns TRUE on success. Failure can only happen due to
 *   invalid parameters.
 */
function intlcal_set_skipped_wall_time_option(IntlCalendar $cal,
                                              int $wallTimeOption): bool {
  return $cal->setSkippedWallTimeOption($wallTimeOption);
}

/**
 * Set the calendar time in milliseconds since the epoch
 *
 * @param IntlCalendar $cal -
 * @param float $date -
 *
 * @return bool - Returns TRUE on success and FALSE on failure.
 */
function intlcal_set_time(IntlCalendar $cal,
                          mixed $date): bool {
  return $cal->setTime($date);
}

/**
 * Set the timezone used by this calendar
 *
 * @param IntlCalendar $cal -
 * @param mixed $timeZone -
 *
 * @return bool - Returns TRUE on success and FALSE on failure.
 */
function intlcal_set_time_zone(IntlCalendar $cal,
                               mixed $timeZone): bool {
  return $cal->setTimezone($timeZone);
}

/**
 * Convert an IntlCalendar into a DateTime object
 *
 * @param IntlCalendar $cal -
 *
 * @return DateTime - A DateTime object with the same timezone as this
 *   object (though using PHP's database instead of ICU's) and the same
 *   time, except for the smaller precision (second precision instead of
 *   millisecond). Returns FALSE on failure.
 */
function intlcal_to_date_time(IntlCalendar $cal): DateTime {
  return $cal->toDateTime();
}

class IntlGregorianCalendar extends IntlCalendar {
   /**
   * Create a new instance of IntlGregorianCalendar
   *
   * Variant 1:
   * @param (TimeZone|IntlTimeZone) $object - Timezone to create from
   * @param string $locale - Locale
   *
   * Variant 2:
   * @param int $year
   * @param int $month
   * @param int $day
   * @param int $hour
   * @param int $minute
   * @param int $second
   */
  <<__Native>>
  public function __construct(mixed $yearOrTz = NULL,
                              mixed $monthOrLocale = NULL,
                              ?int $day = NULL,
                              ?int $hour = NULL,
                              ?int $minute = NULL,
                              ?int $second = NULL): void;

  /**
   * Is the identified year a leap year?
   *
   * @param int $year - A year on the gregorian calendar
   *
   * @return bool - TRUE if a leap year, FALSE otherwise
   */
  <<__Native>>
  public function isLeapYear(int $year): bool;

  /**
   * Gets the Gregorian Calendar change date.
   *
   * This is the point when the switch from Julian dates to
   * Gregorian dates occurred. Default is October 15, 1582.
   * Previous to this, dates will be in the Julian calendar.
   *
   * @return float
   */
  <<__Native>>
  public function getGregorianChange(): float;

  /**
   * Sets the Gregorian Calendar change date.
   *
   * This is the point when the switch from Julian dates to
   * Gregorian dates occurred. Default is October 15, 1582.
   * Previous to this, dates will be in the Julian calendar.
   *
   * @param float
   *
   * @return bool - Whether or not the date was successfully set.
   */
  <<__Native>>
  public function setGregorianChange(float $change): bool;
}

/**
 * Create a new instance of IntlGregorianCalendar
 *
 * Variant 1:
 * @param (TimeZone|IntlTimeZone) $object - Timezone to create from
 * @param string $locale - Locale
 *
 * Variant 2:
 * @param int $year
 * @param int $month
 * @param int $day
 * @param int $hour
 * @param int $minute
 * @param int $second
 *
 * @return IntlGregorianCalendar
 */
function intlgregcal_create_instance(mixed $yearOrTz = NULL,
                                     mixed $monthOrLocale = NULL,
                                     ?int $day = NULL,
                                     ?int $hour = NULL,
                                     ?int $minute = NULL,
                                     ?int $second = NULL)
                                     : IntlGregorianCalendar {
  return new IntlGregorianCalendar($yearOrTz, $monthOrLocale, $day,
                                   $hour, $minute, $second);
}

/**
 * Is the identified year a leap year?
 *
 * @param int $year - A year on the gregorian calendar
 *
 * @return bool - TRUE if a leap year, FALSE otherwise
 */
function intlgregcal_is_leap_year(IntlGregorianCalendar $cal,
                                  int $year): bool {
  return $cal->isLeapYear($year);
}

/**
 * Gets the Gregorian Calendar change date.
 *
 * This is the point when the switch from Julian dates to
 * Gregorian dates occurred. Default is October 15, 1582.
 * Previous to this, dates will be in the Julian calendar.
 *
 * @return float
 */
function intlgregcal_get_gregorian_change(IntlGregorianCalendar $cal): float {
  return $cal->getGregorianChange();
}

/**
 * Sets the Gregorian Calendar change date.
 *
 * This is the point when the switch from Julian dates to
 * Gregorian dates occurred. Default is October 15, 1582.
 * Previous to this, dates will be in the Julian calendar.
 *
 * @param float
 *
 * @return bool - Whether or not the date was successfully set.
 */
function intlgregcal_set_gregorian_change(IntlGregorianCalendar $cal,
                                          float $change): bool {
  return $cal->setGregorianChange($change);
}
