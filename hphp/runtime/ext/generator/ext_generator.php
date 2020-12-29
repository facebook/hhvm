<?hh // partial

<<__NativeData("Generator")>>
final class Generator implements HH\KeyedIterator {

  private function __construct(): void {}

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function current(): mixed;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function key(): mixed;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function next(): mixed;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  private function throw(object $ex): mixed;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function rewind(): mixed;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function valid(): bool;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function send(mixed $v): mixed;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function raise(mixed $v): mixed;

  <<__Native("OpCodeImpl"), __ProvenanceSkipFrame>>
  public function getReturn(): mixed;

  <<__Native>>
  public function getOrigFuncName(): string;

  <<__Native>>
  public function getCalledClass(): string;

}
