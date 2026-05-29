//// /__tests__/tt_classptr_target.php
<?hh

<<file: __PackageOverride('pkg1')>>

// package pkg2 but package pkg1 via packageoverride, in __tests__
class CToTests {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
class DToTests {
  public function bar(): void {
    CToTests::class; // no lint: target is in __tests__
  }
}

class N2 {} // this extra class silences the filename linter
