//// warning_02.php
<?hh

// package pkg2, but pkg1 due to package override
<<file: __PackageOverride('pkg1')>>

class C12 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1 already, so this override is a no-op. It must not buy the file
// an exemption from 5655: the callee's override is still load-bearing.
<<file: __PackageOverride('pkg1')>>

class D12 {
  public function bar(): void {
    (new C12())->foo(); // should raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
