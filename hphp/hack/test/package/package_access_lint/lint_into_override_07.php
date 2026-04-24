//// warning_02.php
<?hh

<<file: __PackageOverride('pkg1')>>

// package pkg2 but package pkg1 via packageoverride
class MyInternType {}

class Extra1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
class WithReifiable {
  <<__Reifiable>>
  const type TIntern = MyInternType;
}

class TestClass {
  public function bar(mixed $x): void {
    // is expression — should NOT trigger Lint[5655]
    $x is MyInternType;
    // ?as expression — should NOT trigger Lint[5655]
    $x ?as MyInternType;
    // as expression — should trigger Lint[5655]
    $x as MyInternType;
  }
}

class Extra2 {} // this extra class silences the filename linter
