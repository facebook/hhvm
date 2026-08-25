//// bar.php
<?hh

// package pkg1, no override
class C11 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// warning_02.php
<?hh

// package pkg2, but pkg3 due to package override. pkg2 already reaches pkg1,
// so the caller's override is not what makes this edge legal and 5655 must not
// fire.
<<file: __PackageOverride('pkg3')>>

class D11 {
  public function bar(): void {
    (new C11())->foo(); // should NOT raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
