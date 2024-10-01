//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TFoo = int;
class Foo {}

//// bar.php
<?hh
// package pkg1
class Bar<reify T> {}
function bar<reify T>(): void {}

function test(): void {
  // No errors
  bar<TFoo>();
  bar<Foo>();
  $_ = new Bar<TFoo>();
  $_ = new Bar<Foo>();
}
