<?php

class DateTimeImmutable implements DateTimeInterface {
  public function __construct(
    string $time = "now",
    DateTimeZone $timezone = null
  ) {
    $this->data = new DateTime($time, $timezone);
  }

  public function add(DateInterval $interval) {
    $out = clone $this;
    $out->data->add($interval);
    return $out;
  }

  public function modify(string $modify) {
    $out = clone $this;
    $out->data->modify($modify);
    return $out;
  }

  public function setDate(int $year, int $month, int $day) {
    $out = clone $this;
    $out->data->setDate($year, $month, $day);
    return $out;
  }

  public function setISODate(
    int $year,
    int $week,
    int $day = 1
  ) {
    $out = clone $this;
    $out->data->setISODate($year, $week, $day);
    return $out;
  }

  public function setTime(
    int $hour,
    int $minute,
    int $second = 0
  ) {
    $out = clone $this;
    $out->data->setTime($hour, $minute, $second);
    return $out;
  }
  public function setTimestamp(int $unixtimestamp) {
    $out = clone $this;
    $out->data->setTimestamp($unixtimestamp);
    return $out;
  }

  public function setTimezone(DateTimeZone $timezone) {
    $out = clone $this;
    $out->data->setTimezone($timezone);
    return $out;
  }

  public function sub(DateInterval $interval) {
    $out = clone $this;
    $out->data->sub($interval);
    return $out;
  }

  public function diff(
    $datetime2,
    $absolute = false
  ) {
    return $this->data->diff($datetime2, $absolute);
  }

  public function format($format) {
    return $this->data->format($format);
  }

  public function getOffset() {
    return $this->data->getOffset();
  }

  public function getTimestamp() {
    return $this->data->getTimestamp();
  }

  public function getTimezone() {
    return $this->data->getTimezone();
  }

  public static function createFromFormat(
    string $format,
    string $time,
    DateTimeZone $timezone = null
  ) {
    $out = new DateTimeImmutable();
    $out->data = DateTime::createFromFormat($format, $time, $timezone);
    return $out;
  }

  public static function getLastErrors() {
    return $this->data->getLastErrors();
  }

  public function __clone() {
    $this->data = clone $this->data;
  }

  private DateTime $data;
}
