<?php

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

  public function __construct(
    DateTimeInterface $start,
    DateInterval $interval,
    mixed $end = null,
    int $options = null) {

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
    } else if ($end instanceof DateTimeInterface) {
      $this->end = clone $end;
    } else {
      throw new Exception(
        "DatePeriod::__construct(): This constructor accepts either " .
        "(DateTimeInterface, DateInterval, int) OR (DateTimeInterface, ".
        "DateInterval, DateTime) as arguments."
      );
    }

    $this->options = $options;
    $this->current = clone $start;
  }

  function current() {
    return $this->current;
  }

  function rewind() {
    $this->current = clone $this->start;

    if ($this->options === DatePeriod::EXCLUDE_START_DATE) {
      $this->next();
    }

    $this->iterKey = 0;
  }

  function key() {
    return $this->iterKey;
  }

  function next() {
    if ($this->valid()) {
      // Assign in case it's a DateTimeImmutable
      $this->current = $this->current->add($this->interval);
      $this->iterKey++;
    }
  }

  function valid() {
    return ($this->current >= $this->start && $this->current < $this->end);
  }

  function getStartDate() {
    return clone $this->start;
  }

  function getEndDate() {
    return clone $this->end;
  }

  function getDateInterval() {
    return clone $this->interval;
  }
}
