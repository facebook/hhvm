<?php

function date_diff(
  DateTimeInterface $datetime,
  DateTimeInterface $datetime2,
  bool $absolute = false
) {
  return $datetime->diff($datetime2, $absolute);
}

function date_format(
  DateTimeInterface $datetime,
  string $format
) {
  return $datetime->format($format);
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
  DateTimeZone $timezone = NULL
) {
  return new DateTimeImmutable($time, $timezone);
}

function timezone_offset_get(
  DateTimeZone $object,
  DateTime $datetime
) {
  return $object->getOffset($datetime);
}
