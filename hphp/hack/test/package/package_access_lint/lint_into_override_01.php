//// warning_02.php
<?hh

// package pkg2, but pkg1 due to package override
<<file: __PackageOverride('pkg1')>>

class C1 {
  <<__RequirePackage('pkg2')>>
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
class D1 {
  public function bar(): void {
    if (package pkg2) {
      (new C1())->foo();
    }
  }
}

class N2 {} // this extra class silences the filename linter
