<?hh /* -*- php -*- */

class IntervalTimer {
  public function __construct(
    float $interval,
    float $initial,
    mixed $callback,
  )[leak_safe];
  public function start()[globals, leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
  public function stop()[globals, leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
}
