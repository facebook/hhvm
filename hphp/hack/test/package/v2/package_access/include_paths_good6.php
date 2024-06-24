//// foo.php
<?hh
// package pkg1
class Foo {};

//// bar.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
class Bar {
  const type TBar = Foo;
}
