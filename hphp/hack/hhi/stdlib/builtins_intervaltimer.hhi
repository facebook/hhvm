<?hh // decl /* -*- php -*- */

class IntervalTimer {
  public function __construct(
    float $interval,
    float $initial,
    mixed $callback,
  );
  public function start();
  public function stop();
}
