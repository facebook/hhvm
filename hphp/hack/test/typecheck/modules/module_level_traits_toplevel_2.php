//// module_A.php
<?hh
new module A {}
//// module_B.php
<?hh
new module B {}

//// A.php
<?hh

<<file: __EnableUnstableFeatures('module_level_traits')>>

// basic module level traits behaviour:
// - T belongs to module A even if used by class C in B
// - getFoo cannot invoke foo which belongs to B

module A;

<<__ModuleLevelTrait>>
public trait T {
  public function getFoo(): void {
    foo();
  }
}

//// B.php
<?hh

module B;

internal function foo(): void { echo "foo\n"; }

class C {
  use T;
}

<<__EntryPoint>>
function bar(): void {
  (new C())->getFoo();
}
