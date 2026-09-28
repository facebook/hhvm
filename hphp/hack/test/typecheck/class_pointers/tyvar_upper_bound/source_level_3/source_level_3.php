<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

newtype ClassPointer = class<C>;

function newtype_source<T super ClassPointer as string>(?T $_ = null): T {
  throw new Exception();
}

function test_newtype(): void {
  $result = newtype_source();
  $result::method();
}
