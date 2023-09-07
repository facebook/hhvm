<?hh // partial

/**
 * A timer that periodically interrupts a request thread.
 */
<<__NativeData>>
class IntervalTimer {
  /**
   * Create a new interval timer.
   *
   * @param double $interval - frequency in seconds of timer interrupts.
   */
  <<__Native>>
  public function __construct(float $interval,
                              float $initial,
                              mixed $callback)[leak_safe];

  /**
   * Start the timer.
   */
  <<__Native>>
  public function start()[globals, leak_safe]: void;

  /**
   * Stop the timer.
   */
  <<__Native>>
  public function stop()[globals, leak_safe]: void;
}
