//// warning_cc_04.php
<?hh

// package pkg2
class C2 {}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
<<__Sealed(C2::class)>> // linter should not fire on attribute
class C1 {
  public function foo(): void {
    $c = C2::class;  // sanity check: linter should fire here
  }
}

class N2 {} // this extra class silences the filename linter
