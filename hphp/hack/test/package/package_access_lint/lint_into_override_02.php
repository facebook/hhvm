//// warning_02.php
<?hh

<<file: __PackageOverride('pkg1')>>

// package pkg2 but package pkg1 via packageoverride
class C2 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
class D2 {
  public function bar(): void {
    (new C2())->foo(); // should raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
