//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
class BarImpl {};
type TFoo1 = int;
newtype TFoo2 = string;

//// bar.php
<?hh
// package pkg1

class Bar {
  const type T1 = BarImpl;
  const type T2 = TFoo1;
  const type T3 = TFoo2;
}
