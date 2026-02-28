//// warning_03a.php
<?hh

<<file: __PackageOverride('pkg1')>>

// package pkg3 but package pkg1 via packageoverride
class C3 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// warning_03b.php
<?hh

// package pkg2
class D3 {
  public function bar(): void {
    (new C3())->foo(); // should raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
