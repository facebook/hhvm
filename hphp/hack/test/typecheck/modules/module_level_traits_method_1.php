//// module_A.php
<?hh
new module A {}

//// module_B.php
<?hh
new module B {}

//// A.php
<?hh

// basic module level traits behaviour:
// - T belongs to module A even if used by class C in B
// - getFoo can thus safely invoke foo

module A;

class D {
  internal function foo(): void { echo "foo\n"; }
}

<<__ModuleLevelTrait>>
public trait T {
  public function getFoo(D $d): void {
    $d->foo();
  }
}

//// B.php
<?hh

module B;

class C {
  use T;

  public function bar(): void {
    $d = new D();
    $this->getFoo($d);
  }
}

<<__EntryPoint>>
function main(): void {
  (new C())->bar();
}
