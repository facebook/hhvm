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
    DateTime $start,
    DateInterval $interval = null,
    mixed $end = null,
    int $options = null) {

    $this->start = clone $start;
    $this->interval = clone $interval;

    if (is_int($end)) {
      // $end is really $recurrances
      $this->recurrances = $end;
      $end_date = clone $start;
      for ($i = 0; $i < $this->recurrances; $i++) {
        // add the interval to the start date 'n' times
        $end_date->add($interval);
      }
      $this->end = $end_date;
    } else {
      $this->end = $end;
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
      $this->current->add($this->interval);
      $this->iterKey++;
    }
  }

  function valid() {
    return ($this->current >= $this->start && $this->current <= $this->end);
  }
}
