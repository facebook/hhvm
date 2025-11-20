//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
class Foo {}

//// bar.php
<?hh
// package pkg1
function bar(): void {
  $foo = new Foo();
}
