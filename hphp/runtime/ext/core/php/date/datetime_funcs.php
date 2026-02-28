<?hh

function date_diff(
  DateTimeInterface $datetime,
  DateTimeInterface $datetime2,
  bool $absolute = false
) {
  return $datetime->diff($datetime2, $absolute);
}

function date_timezone_get(DateTimeInterface $datetime) {
  return $datetime->getTimezone();
}

function date_offset_get(DateTimeInterface $datetime) {
  return $datetime->getOffset();
}

function date_timestamp_get(DateTimeInterface $datetime) {
  return $datetime->getTimestamp();
}

function date_create_immutable(
  string $time = 'now',
  ?DateTimeZone $timezone = NULL
) {
  try {
    return new DateTimeImmutable($time, $timezone);
  } catch (Exception $ex) {
    // DateTime and DateTimeImmutable both throw exceptions when they fail but
    // date_create and date_create_immutable should both simply return false
    return false;
  }
}

function timezone_offset_get(
  DateTimeZone $object,
  DateTime $datetime
) {
  return $object->getOffset($datetime);
}
