<?hh // decl    /* -*- php -*- */

class IntervalTimer {
  public function __construct(float $interval,
                              float $initial,
                              callable $callback);
  public function start();
  public function stop();
}
