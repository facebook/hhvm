//// warning_cc_04.php
<?hh

// package pkg2
class C2 {
  const int FOO = 3;
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
class D2 {
  public function bar(): void {
    C2::FOO; // should NOT raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
