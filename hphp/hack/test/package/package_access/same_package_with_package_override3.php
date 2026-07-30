//// pkg6/foo.php
<?hh
<<file: __PackageOverride('pkg1')>>
class Foo {}

//// pkg6/bar.php
<?hh
<<file: __PackageOverride('pkg1')>>
function bar(): void {
  $foo = new Foo();
}
