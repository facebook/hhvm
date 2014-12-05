<?php

/* Type hints are missing as any classes that implements this interface must
 * specify types, meaning that no classes written in PHP can implement this
 * interface if type hints are included.
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
