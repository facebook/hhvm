//// bar.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
class MyInternClass {}

//// foo.php
<?hh
// package pkg1

abstract class MyProdBase {
  abstract const type T1;
  <<__Reifiable>> abstract const type T2;
}

class MyProd extends MyProdBase {
  const type T1 = MyInternClass; // ok
  const type T2 = MyInternClass; // error when package_v2_allow_reifiable_tconst_violations is off
}
