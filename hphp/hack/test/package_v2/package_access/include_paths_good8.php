//// foo.php
<?hh
// package pkg1
type TFoo = int;

enum EFoo: int {
  ZERO = 0;
  ONE = 1;
}

final class CFoo implements HH\ClassAttribute {}

//// bar.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>

type A = shape(
  'bar' => string,
  'foo' => TFoo,
);
type B = vec<TFoo>;
<<CFoo()>> class C {}
type D = dict<string, TFoo>;
type F1 = (function(TFoo): int);
type F2 = (function(int): TFoo);

interface I<T> {
  public function get(): T;
}
trait T implements I<?EFoo> {
  private ?EFoo $value;
  public function get(): ?EFoo {
    return $this->value;
  }
}
