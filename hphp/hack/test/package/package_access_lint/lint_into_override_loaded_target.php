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

// package pkg1. pkg1 does NOT include pkg2; it reaches pkg2 only through the
// runtime `package pkg2` check below, not structurally. pkg2 does not include
// the callee's underlying package pkg3, so loading pkg2 does not make pkg3
// reachable either. The override is therefore load-bearing and 5655 fires.
// The removed `package_includes current_pkg target_pkg` guard would have
// silenced this, because pkg1 does not include the override target pkg2.
class D {
  public function bar(): void {
    if (package pkg2) {
      (new C())->foo(); // should raise a lint error
    }
  }
}

class N2 {} // this extra class silences the filename linter
