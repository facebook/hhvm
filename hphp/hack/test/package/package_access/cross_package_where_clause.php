//// bar.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
class MyInternClass {
  const type MyInternType = int;
}

//// foo.php
<?hh
// package pkg1

class MyProdClass {
  public function MyProdMethod<T>(T $x): T
  where
    // No errors
    T = MyInternClass::MyInternType {
    return $x;
  }
}

function MyProdFunction<T>(T $x): T
where
  // No errors
  T = MyInternClass::MyInternType {
  return $x;
}
