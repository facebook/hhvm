<?hh

<<__NativeData>>
final class Generator implements HH\KeyedIterator {

  private function __construct(): void {}

  <<__Native("OpCodeImpl")>>
  public function current()[]: mixed;

  <<__Native("OpCodeImpl")>>
  public function key()[]: mixed;

  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  public function next()[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  private function throw(Throwable $ex)[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  public function rewind()[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl")>>
  public function valid()[]: bool;

  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  public function send(mixed $v)[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl"), __NEVER_INLINE>>
  public function raise(mixed $v)[/* gen $this */]: mixed;

  <<__Native("OpCodeImpl")>>
  public function getReturn()[]: mixed;

  <<__Native>>
  public function getOrigFuncName()[]: string;

  <<__Native>>
  public function getCalledClass()[]: string;

}
