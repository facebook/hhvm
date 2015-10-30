<?hh

<<__NativeData("Generator")>>
final class Generator implements HH\KeyedIterator {

  private function __construct(): void {}

  <<__Native("OpCodeImpl")>>
  function current(): mixed;

  <<__Native("OpCodeImpl")>>
  function key(): mixed;

  <<__Native("OpCodeImpl")>>
  function next(): mixed;

  <<__Native("OpCodeImpl")>>
  private function throw(object $ex): mixed;

  <<__Native("OpCodeImpl")>>
  function rewind(): mixed;

  <<__Native("OpCodeImpl")>>
  function valid(): bool;

  <<__Native("OpCodeImpl")>>
  function send(mixed $v): mixed;

  <<__Native("OpCodeImpl")>>
  function raise(mixed $v): mixed;

  <<__Native("OpCodeImpl")>>
  function getReturn(): mixed;

  <<__Native>>
  function getOrigFuncName(): string;

  <<__Native>>
  function getCalledClass(): string;

}
