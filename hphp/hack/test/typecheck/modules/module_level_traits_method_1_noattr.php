//// module_A.php
<?hh
new module A {}
//// module_B.php
<?hh
new module B {}

//// A.php
<?hh

// T is not a module level trait, so the standard semantics applies:
// public trait methods cannot invoke internal functions

module A;

class D {
  internal function foo(): void { echo "foo\n"; }
}

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
