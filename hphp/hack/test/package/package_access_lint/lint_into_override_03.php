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

// package pkg2. C3's original package is pkg3, which pkg2 cannot reach; the
// override into pkg1 (which pkg2 includes) is the only reason this edge is
// legal, so 5655 fires: without the override pkg2 could not access C3.
class D3 {
  public function bar(): void {
    (new C3())->foo(); // should raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
