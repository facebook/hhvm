<?hh

class DateTimeImmutable implements DateTimeInterface {
  public function __construct(
    string $time = "now",
    ?DateTimeZone $timezone = null
  ) {
    $this->data = new DateTime($time, $timezone);
  }

  public function add(DateInterval $interval): DateTimeImmutable {
    $out = clone $this;
    $out->data->add($interval);
    return $out;
  }

  public function modify(string $modify): DateTimeImmutable {
    $out = clone $this;
    $out->data->modify($modify);
    return $out;
  }

  public function setDate(int $year, int $month, int $day): DateTimeImmutable {
    $out = clone $this;
    $out->data->setDate($year, $month, $day);
    return $out;
  }

  public function setISODate(
    int $year,
    int $week,
    int $day = 1
  ): DateTimeImmutable {
    $out = clone $this;
    $out->data->setISODate($year, $week, $day);
    return $out;
  }

  public function setTime(
    int $hour,
    int $minute,
    int $second = 0
  ): DateTimeImmutable {
    $out = clone $this;
    $out->data->setTime($hour, $minute, $second);
    return $out;
  }
  public function setTimestamp(int $unixtimestamp): DateTimeImmutable {
    $out = clone $this;
    $out->data->setTimestamp($unixtimestamp);
    return $out;
  }

  public function setTimezone(DateTimeZone $timezone): DateTimeImmutable {
    $out = clone $this;
    $out->data->setTimezone($timezone);
    return $out;
  }

  public function sub(DateInterval $interval): DateTimeImmutable {
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

  public function getOffset(): int {
    return $this->data->getOffset();
  }

  public function getTimestamp()[]: int {
    return $this->data->getTimestamp();
  }

  public function getTimezone() {
    return $this->data->getTimezone();
  }

  public static function createFromFormat(
    string $format,
    string $time,
    ?DateTimeZone $timezone = null
  ): mixed {
    $out = new DateTimeImmutable();
    $data = DateTime::createFromFormat($format, $time, $timezone);
    if ($data === false) {
      return false;
    }
    $out->data = $data;
    return $out;
  }

  public static function createFromMutable(DateTime $datetime) {
    $out = new DateTimeImmutable();
    $out->data = clone $datetime;
    return $out;
  }

  public static function getLastErrors(): darray {
    return DateTime::getLastErrors();
  }

  public function __clone() {
    $this->data = clone $this->data;
  }

  private DateTime $data;
}
