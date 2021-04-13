<?hh // partial

<<__NativeData("Generator")>>
final class Generator implements HH\KeyedIterator {

  private function __construct(): void {}

  <<__Native("OpCodeImpl")>>
  public function current()[]: mixed;

  <<__Native("OpCodeImpl")>>
  public function key()[]: mixed;

  <<__Native("OpCodeImpl")>>
  public function next()[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl")>>
  private function throw(object $ex)[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl")>>
  public function rewind()[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl")>>
  public function valid()[]: bool;

  <<__Native("OpCodeImpl")>>
  public function send(mixed $v)[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl")>>
  public function raise(mixed $v)[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl")>>
  public function getReturn()[]: mixed;

  <<__Native>>
  public function getOrigFuncName()[]: string;

  <<__Native>>
  public function getCalledClass()[]: string;

}
