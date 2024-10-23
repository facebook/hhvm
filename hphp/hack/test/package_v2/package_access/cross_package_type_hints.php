//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TFoo = int;
newtype NTFoo = string;
class Foo {}

//// bar.php
<?hh
// package pkg1

// All ok:
function test1(TFoo  $_) : void {}
function test2(NTFoo $_) : void {}
function test3(Foo   $_) : void {}
class Bar {
  const TFoo ClsConst = 42;
}
const TFoo GlobalConst = 42;
