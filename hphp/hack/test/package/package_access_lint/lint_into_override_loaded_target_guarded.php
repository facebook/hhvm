//// warning_03a.php
<?hh

// physically package pkg3, pulled into pkg2 via override
<<file: __PackageOverride('pkg2')>>

class C {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1. Same loaded-only-target setup as
// lint_into_override_loaded_target (pkg1 reaches the override target pkg2
// only via a runtime check), but the call is additionally guarded by
// `package pkg3`, the callee's underlying package. With the underlying
// package reachable the override is no longer load-bearing, so 5655 must not
// fire.
class D {
  public function bar(): void {
    if (package pkg2) {
      if (package pkg3) {
        (new C())->foo(); // should NOT raise a lint error
      }
    }
  }
}

class N2 {} // this extra class silences the filename linter
