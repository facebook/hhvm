<?hh

final class C implements HH\MethodAttribute {}

type Alias = C;

final class Foo<T> {
  // Reserved user attribute
  <<__Memoize>>
  public function bar(): void {}

  <<C>>
  public function baz(): void {}

  // Not allowed
  <<NotFound>>
  public function qux(): void {}

  // No generics allowed
  <<T>>
  public function zab(): void {}

  // No typedefs allowed
  <<Alias>>
  public function nar(): void {}
}
