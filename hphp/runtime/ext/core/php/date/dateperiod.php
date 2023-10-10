<?hh

class DatePeriod implements Iterator {

  const EXCLUDE_START_DATE = 1;

  private
    $start = null,
    $interval = null,
    $end = null,
    $options = null,
    $current = null,
    $recurrances = null,
    $iterKey = 0;

  public function __construct(mixed ...$args) {
    $count = count($args);
    if ($count === 1 || $count === 2) {
      $isostr = $args[0];
      $options = $args[1] ?? 0;
      if (!(is_string($isostr) && is_int($options))) {
        $this->__throwConstructorUsageException();
      }
      // ISO: R[n]/start/interval
      // PHP: n is required
      $parts = explode('/', $isostr);
      if (count($parts) !== 3) {
        throw new Exception(
          'The ISO interval did not contain an end date or recurrence count'
        );
      }
      $recurrences = substr($parts[0], 1);
      if ($recurrences === '' || !ctype_digit($recurrences)) {
        throw new Exception(
          'The ISO interval did not contain a recurrence count'
        );
      }
      $recurrences = (int) $recurrences;
      $this->__constructImpl(
        new DateTime($parts[1]),
        new DateInterval($parts[2]),
        $recurrences,
        $options,
      );

      return;
    }

    if ($count > 2) {
      $this->__constructImpl(...$args);
      return;
    }

    $this->__throwConstructorUsageException();
  }

  private function __throwConstructorUsageException(): void {
    throw new Exception(
      "DatePeriod::__construct(): This constructor accepts either " .
      "(DateTimeInterface, DateInterval, int) OR (DateTimeInterface, ".
      "DateInterval, DateTime) OR (string[, int]) as arguments."
    );
  }

  private function __constructImpl(
    DateTimeInterface $start,
    DateInterval $interval,
    mixed $end = null,
    ?int $options = null): void {

    $this->start = clone $start;
    $this->interval = clone $interval;

    if (is_int($end)) {
      // $end is really $recurrances
      $this->recurrances = $end;
      $end_date = clone $start;
      for ($i = 0; $i <= $this->recurrances; $i++) {
        // add the interval to the start date 'n' times
        $end_date->add($interval);
      }
      $this->end = $end_date;
    } else if ($end is DateTimeInterface) {
      $this->end = clone $end;
    } else {
      $this->__throwConstructorUsageException();
    }

    $this->options = $options;
    $this->current = clone $start;
  }

  public function current() {
    return clone $this->current;
  }

  public function rewind() {
    $this->current = clone $this->start;

    if ($this->options === DatePeriod::EXCLUDE_START_DATE) {
      $this->next();
    }

    $this->iterKey = 0;
  }

  public function key() {
    return $this->iterKey;
  }

  public function next() {
    if ($this->valid()) {
      $current = clone $this->current;
      $this->current = $current->add($this->interval);
      $this->iterKey++;
    }
  }

  public function valid(): bool {
    return ($this->current >= $this->start && $this->current < $this->end);
  }

  public function getStartDate() {
    return clone $this->start;
  }

  public function getEndDate() {
    return clone $this->end;
  }

  public function getDateInterval() {
    return clone $this->interval;
  }
}
