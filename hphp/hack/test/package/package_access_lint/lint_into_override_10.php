//// warning_03a.php
<?hh

// package pkg3, no override
class C10 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// warning_02.php
<?hh

// package pkg2, but pkg3 due to package override. Only the caller is
// overridden, and its override is what makes this edge legal: pkg2 cannot
// reach pkg3, so 5655 fires.
<<file: __PackageOverride('pkg3')>>

class D10 {
  public function bar(): void {
    (new C10())->foo(); // should raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
