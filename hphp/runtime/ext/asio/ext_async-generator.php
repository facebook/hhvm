<?hh

namespace HH {

<<__NativeData>>
final class AsyncGenerator<+Tk, +Tv, -Ts> implements AsyncKeyedIterator<Tk, Tv> {

  private function __construct(): void {}

  /**
   * Continue asynchronous iteration
   */
  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  public function next()[/* gen $this */]: Awaitable<?(Tk, Tv)>;

  /**
   * Continue asynchronous iteration with value
   * @param mixed $value - A value to be received by yield expression
   */
  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  public function send(?Ts $value)[/* gen $this */]: mixed;

  /**
   * Continue asynchronous iteration with raised exception
   * @param Exception $exception - An exception to be thrown by yield expression
   */
  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  public function raise(
    \Exception $exception,
  )[/* gen $this */]: Awaitable<?(Tk, Tv)>;
}

}
