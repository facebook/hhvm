<?hh

/**
 * A timer that periodically interrupts a request thread.
 */
<<__NativeData("IntervalTimer")>>
class IntervalTimer {
  /**
   * Create a new interval timer.
   *
   * @param double $interval - frequency in seconds of timer interrupts.
   */
  <<__Native>>
  public function __construct(double $interval,
                              double $initial,
                              mixed $callback);

  /**
   * Start the timer.
   */
  <<__Native>>
  public function start(): void;

  /**
   * Stop the timer.
   */
  <<__Native>>
  public function stop(): void;
}
