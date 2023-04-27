//// a.module.php
<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    B
  }
}

//// b.module.php
<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module B { }

//// a.php
<?hh


module A;

class A implements IB {
  public function foo(): void {
  }
}

public function getA(): IB {
  return new A();
}

//// b.php
<?hh


module B;

interface IB {
  public function foo(): void;
}

public function test(): void {
  $x = getA();
  $x->foo(); // ok
}
