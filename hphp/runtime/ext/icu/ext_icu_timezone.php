<?hh

<<__NativeData>>
class IntlTimeZone {

  /**
   * Objects are only created wit the static create functions
   */
  public final function __construct() {
    throw new Exception(
      "An object of this type cannot be created with the new operator",
    );
  }

  /**
   * Get the number of IDs in the equivalency group that includes the given
   * ID
   *
   * @param string $zoneId -
   *
   * @return integer -
   */
  <<__Native>>
  public static function countEquivalentIDs(string $zoneId): mixed;

  <<__Native>>
  public static function createTimeZoneIDEnumeration(
    int $zonetype,
    string $region = "",
    ?int $offset = null,
  ): mixed;

  /**
   * Create a new copy of the default timezone for this host
   *
   * @return IntlTimeZone -
   */
  <<__Native>>
  public static function createDefault(): IntlTimeZone;

  /**
   * Get an enumeration over time zone IDs associated with the
   *   given country or offset
   *
   * @param mixed $countryOrRawOffset -
   *
   * @return IntlIterator -
   */
  <<__Native>>
  public static function createEnumeration(
    mixed $countryOrRawOffset = null,
  ): mixed;

  /**
   * Create a timezone object for the given ID
   *
   * @param string $zoneId -
   *
   * @return IntlTimeZone -
   */
  <<__Native>>
  public static function createTimeZone(string $zoneId): IntlTimeZone;

  /**
   * Create a timezone object from
   *
   * @param DateTimeZone $zoneId -
   *
   * @return IntlTimeZone -
   */
  public static function fromDateTimeZone(DateTimeZone $zoneId): IntlTimeZone {
    return self::createTimeZone($zoneId->getname());
  }

  /**
   * Get the canonical system timezone ID or the normalized custom time zone
   * ID for the given time zone ID
   *
   * @param string $zoneId -
   * @param bool $isSystemID -
   *
   * @return string -
   */
  <<__Native>>
  public static function getCanonicalID(
    string $zoneId,
    <<__OutOnly("KindOfBoolean")>> inout mixed $isSystemID,
  ): mixed;


  <<__Native>>
  public static function getRegion(string $str): mixed;

  /**
   * Get a name of this time zone suitable for presentation to the user
   *
   * @param bool $isDaylight -
   * @param integer $style -
   * @param string $locale -
   *
   * @return string -
   */
  <<__Native>>
  public function getDisplayName(
    bool $isDaylight = false,
    int $style = IntlTimeZone::DISPLAY_LONG,
    string $locale = "",
  ): mixed;

  /**
   * Get the amount of time to be added to local standard time to get local
   * wall clock time
   *
   * @return integer -
   */
  <<__Native>>
  public function getDSTSavings(): int;

  /**
   * Get an ID in the equivalency group that includes the given ID
   *
   * @param string $zoneId -
   * @param integer $index -
   *
   * @return string -
   */
  <<__Native>>
  public static function getEquivalentID(string $zoneId, int $index): mixed;

  /**
   * Get last error code on the object
   *
   * @return integer -
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get last error message on the object
   *
   * @return string -
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Create GMT (UTC) timezone
   *
   * @return IntlTimeZone -
   */
  <<__Native>>
  public static function getGMT(): IntlTimeZone;

  <<__Native>>
  public static function getUnknown(): IntlTimeZone;

  /**
   * Get timezone ID
   *
   * @return string -
   */
  <<__Native>>
  public function getID(): mixed;

  /**
   * Get the time zone raw and GMT offset for the given moment in time
   *
   * @param float $date -
   * @param bool $local -
   * @param integer $rawOffset -
   * @param integer $dstOffset -
   *
   * @return integer -
   */
  <<__Native>>
  public function getOffset(
    float $date,
    bool $local,
    <<__OutOnly('KindOfInt64')>> inout int $rawOffset,
    <<__OutOnly('KindOfInt64')>> inout int $dstOffset,
  ): bool;

  /**
   * Get the raw GMT offset (before taking daylight savings time into
   * account
   *
   * @return integer -
   */
  <<__Native>>
  public function getRawOffset(): mixed;

  /**
   * Get the timezone data version currently used by ICU
   *
   * @return string -
   */
  <<__Native>>
  public static function getTZDataVersion(): mixed;

  /**
   * Check if this zone has the same rules and offset as another zone
   *
   * @param IntlTimeZone $otherTimeZone -
   *
   * @return bool -
   */
  <<__Native>>
  public function hasSameRules(IntlTimeZone $otherTimeZone): bool;

  /**
   * Convert to  object
   *
   * @return DateTimeZone -
   */
  public function toDateTimeZone(): DateTimeZone {
    $id = $this->getID();
    if (!strncmp($id, "GMT", 3)) {
      throw new Exception(
        "Converting IntlTimeZone to DateTimeZone ".
        "is not currently supported for GMT offsets",
      );
    }
    return new DateTimeZone($id);
  }

  /**
   * Check if this time zone uses daylight savings time
   *
   * @return bool -
   */
  <<__Native>>
  public function useDaylightTime(): bool;
}

/**
 * Get the number of IDs in the equivalency group that includes the given
 * ID
 *
 * @param string $zoneId -
 *
 * @return integer -
 */
function intltz_count_equivalent_ids(string $zoneId): mixed {
  return IntlTimeZone::countEquivalentIDs($zoneId);
}

function intltz_create_time_zone_id_enumeration(
  int $zonetype,
  string $region = "",
  ?int $offset = null,
): mixed {
  return IntlTimeZone::createTimeZoneIDEnumeration($zonetype, $region, $offset);
}

/**
 * Create a new copy of the default timezone for this host
 *
 * @return IntlTimeZone -
 */
function intltz_create_default(): IntlTimeZone {
  return IntlTimeZone::createDefault();
}

/**
 * Get an enumeration over time zone IDs associated with the
 *   given country or offset
 *
 * @param mixed $countryOrRawOffset -
 *
 * @return IntlIterator -
 */
function intltz_create_enumeration(mixed $countryOrRawOffset = null): mixed {
  return IntlTimeZone::createEnumeration($countryOrRawOffset);
}

/**
 * Create a timezone object for the given ID
 *
 * @param string $zoneId -
 *
 * @return IntlTimeZone -
 */
function intltz_create_time_zone(string $zoneId): IntlTimeZone {
  return IntlTimeZone::createTimeZone($zoneId);
}

/**
 * Create a timezone object from
 *
 * @param DateTimeZone $zoneId -
 *
 * @return IntlTimeZone -
 */
function intltz_from_date_time_zone(DateTimeZone $zoneId): IntlTimeZone {
  return IntlTimeZone::fromDateTimeZone($zoneId);
}

/**
 * Get the canonical system timezone ID or the normalized custom time zone
 * ID for the given time zone ID
 *
 * @param string $zoneId -
 * @param bool $isSystemID -
 *
 * @return string -
 */
function intltz_get_canonical_id(
  string $zoneId,
  inout mixed $isSystemID,
): mixed {
  return IntlTimeZone::getCanonicalID($zoneId, inout $isSystemID);
}

function intltz_get_region(string $str): mixed {
  return IntlTimeZone::getRegion($str);
}

/**
 * Get a name of this time zone suitable for presentation to the user
 *
 * @param bool $isDaylight -
 * @param integer $style -
 * @param string $locale -
 *
 * @return string -
 */
function intltz_get_display_name(
  IntlTimeZone $obj,
  bool $isDaylight = false,
  int $style = IntlTimeZone::DISPLAY_LONG,
  string $locale = "",
): mixed {
  return $obj->getDisplayName($isDaylight, $style, $locale);
}

/**
 * Get the amount of time to be added to local standard time to get local
 * wall clock time
 *
 * @return integer -
 */
function intltz_get_dst_savings(IntlTimeZone $obj): int {
  return $obj->getDSTSavings();
}

/**
 * Get an ID in the equivalency group that includes the given ID
 *
 * @param string $zoneId -
 * @param integer $index -
 *
 * @return string -
 */
function intltz_get_equivalent_id(string $zoneId, int $index): mixed {
  return IntlTimeZone::getEquivalentID($zoneId, $index);
}

/**
 * Get last error code on the object
 *
 * @return integer -
 */
function intltz_get_error_code(IntlTimeZone $obj): int {
  return $obj->getErrorCode();
}

/**
 * Get last error message on the object
 *
 * @return string -
 */
function intltz_get_error_message(IntlTimeZone $obj): string {
  return $obj->getErrorMessage();
}

/**
 * Create GMT (UTC) timezone
 *
 * @return IntlTimeZone -
 */
function intltz_get_gmt(): IntlTimeZone {
  return IntlTimeZone::getGMT();
}

function intltz_get_unknown(): IntlTimeZone {
  return IntlTimeZone::getUnknown();
}

/**
 * Get timezone ID
 *
 * @return string -
 */
function intltz_get_id(IntlTimeZone $obj): string {
  return $obj->getID();
}

/**
 * Get the time zone raw and GMT offset for the given moment in time
 *
 * @param float $date -
 * @param bool $local -
 * @param integer $rawOffset -
 * @param integer $dstOffset -
 *
 * @return integer -
 */
function intltz_get_offset(
  IntlTimeZone $obj,
  float $date,
  bool $local,
  inout int $rawOffset,
  inout int $dstOffset,
): bool {
  return $obj->getOffset($date, $local, inout $rawOffset, inout $dstOffset);
}

/**
 * Get the raw GMT offset (before taking daylight savings time into
 * account
 *
 * @return integer -
 */
function intltz_get_raw_offset(IntlTimeZone $obj): mixed {
  return $obj->getRawOffset();
}

/**
 * Get the timezone data version currently used by ICU
 *
 * @return string -
 */
function intltz_get_tz_data_version(): mixed {
  return IntlTimeZone::getTZDataVersion();
}

/**
 * Check if this zone has the same rules and offset as another zone
 *
 * @param IntlTimeZone $otherTimeZone -
 *
 * @return bool -
 */
function intltz_has_same_rules(
  IntlTimeZone $obj,
  IntlTimeZone $otherTimeZone,
): bool {
  return $obj->hasSameRules($otherTimeZone);
}

/**
 * Convert to  object
 *
 * @return DateTimeZone -
 */
function intltz_to_date_time_zone(IntlTimeZone $obj): DateTimeZone {
  return $obj->toDateTimeZone();
}

/**
 * Check if this time zone uses daylight savings time
 *
 * @return bool -
 */
function intltz_use_daylight_time(IntlTimeZone $obj): bool {
  return $obj->useDaylightTime();
}
