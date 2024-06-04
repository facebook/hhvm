//// foo.php
<?hh
// package pkg2
<<file: __EnableUnstableFeatures('package_v2'), __PackageOverride('pkg2')>>
class Foo {}

//// bar.php
<?hh
// package pkg1
function bar(): void {
  $foo = new Foo();
}
