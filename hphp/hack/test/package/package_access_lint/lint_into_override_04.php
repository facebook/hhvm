//// warning_02.php
<?hh

<<file: __PackageOverride('pkg1')>>

// package pkg2 but package pkg1 via packageoverride
class C2 {
  const int FOO = 3;
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
class D2 {
  public function bar(): void {
    C2::FOO; // should raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
