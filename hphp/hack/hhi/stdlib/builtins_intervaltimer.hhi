<?hh // decl /* -*- php -*- */

class IntervalTimer {
  public function __construct(
    float $interval,
    float $initial,
    (function (string): void) $callback,
  );
  public function start();
  public function stop();
}
