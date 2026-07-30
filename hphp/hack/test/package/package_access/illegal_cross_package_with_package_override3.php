//// pkg6/foo.php
<?hh
<<file: __PackageOverride('pkg1')>>
class Foo {}

//// pkg4/bar.php
<?hh
function bar(): void {
  $foo = new Foo();
}
