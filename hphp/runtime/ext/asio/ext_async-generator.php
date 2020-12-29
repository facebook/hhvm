<?hh // partial

namespace HH {

<<__NativeData("HH\\AsyncGenerator")>>
final class AsyncGenerator implements AsyncKeyedIterator {

  private function __construct(): void {}

  /**
   * Continue asynchronous iteration
   */
  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function next(): mixed;

  /**
   * Continue asynchronous iteration with value
   * @param mixed $value - A value to be received by yield expression
   */
  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function send(mixed $value): mixed;

  /**
   * Continue asynchronous iteration with raised exception
   * @param Exception $exception - An exception to be thrown by yield expression
   */
  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function raise(\Exception $exception): mixed;
}

}
