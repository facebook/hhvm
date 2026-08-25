//// warning_03a.php
<?hh

// package pkg3, but pkg1 due to package override
<<file: __PackageOverride('pkg1')>>

class C8 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// warning_02.php
<?hh

// package pkg2, but pkg1 due to package override. Both files land in pkg1 only
// because of their overrides; pkg2 cannot reach pkg3, so removing them turns
// this edge into an error and 5655 fires.
<<file: __PackageOverride('pkg1')>>

class D8 {
  public function bar(): void {
    (new C8())->foo(); // should raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
