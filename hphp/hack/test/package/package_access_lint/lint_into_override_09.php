//// warning_02.php
<?hh

// package pkg2, but pkg1 due to package override
<<file: __PackageOverride('pkg1')>>

class C9 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// warning_03a.php
<?hh

// package pkg3, but pkg1 due to package override. pkg3 includes pkg2, so this
// edge survives removing both overrides and 5655 must not fire.
<<file: __PackageOverride('pkg1')>>

class D9 {
  public function bar(): void {
    (new C9())->foo(); // should NOT raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
