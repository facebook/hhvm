<?php

/* Type hints are missing as DateTime is IDL + C++ - with the type hints,
 * the C++ implementation is considered incompatible.
 */
interface DateTimeInterface {
  public function diff(
    /* DateTimeInterface */ $datetime2,
    /* bool */ $absolute /*= false */
  );
  public function format(/* string */ $format);
  public function getOffset();
  public function getTimestamp();
  public function getTimezone();
}
