<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

newtype ClassPointer = class<C>;

function id_string<T as string>(T $value): T {
  return $value;
}

function test_newtype(ClassPointer $class): void {
  $result = id_string($class);
  $result::method();
}
