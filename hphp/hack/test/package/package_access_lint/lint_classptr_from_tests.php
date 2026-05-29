//// warning_cc_01.php
<?hh

<<file: __PackageOverride('pkg1')>>

// package pkg2 but package pkg1 via packageoverride
class CFromTests {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// /__tests__/test_classptr_source.php
<?hh

// package pkg1 but in __tests__, should be excluded
class DFromTests {
  public function bar(): void {
    CFromTests::class; // no lint: source is in __tests__
  }
}

class N2 {} // this extra class silences the filename linter
