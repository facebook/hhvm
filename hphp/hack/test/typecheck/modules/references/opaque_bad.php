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

new module B {
  exports {
    C
  }
}

//// c.module.php
<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module C {}

//// a.php
<?hh


module A;

public class A {
  public function foo(): void {
  }
}

//// b.php
<?hh


module B;

public class B {
  public function getA(): A {
    return new A();
  }
}

//// c.php
<?hh


module C;

public function test(): void {
  $x = new B();
  $y = $x->getA();
  // TODO: This should fail, coming soon...
  $y->foo();
}
