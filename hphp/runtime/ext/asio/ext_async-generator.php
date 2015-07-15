<?hh

namespace HH {

<<__NativeData("HH\\AsyncGenerator")>>
final class AsyncGenerator implements AsyncKeyedIterator {

  private function __construct(): void {}

  /**
   * Continue asynchronous iteration
   */
  <<__Native("OpCodeImpl")>>
  function next(): mixed;

  /**
   * Continue asynchronous iteration with value
   * @param mixed $value - A value to be received by yield expression
   */
  <<__Native("OpCodeImpl")>>
  function send(mixed $value): mixed;

  /**
   * Continue asynchronous iteration with raised exception
   * @param Exception $exception - An exception to be thrown by yield expression
   */
  <<__Native("OpCodeImpl")>>
  function raise(Exception $exception): mixed;
}

}
