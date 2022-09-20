//// a.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    B
  }
}

//// b.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module B {
  exports {
    C
  }
}

//// c.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module C {}

//// a.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A;

public class A {
  public function foo(): void {
  }
}

//// b.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module B;

public class B {
  public function getA(): A {
    return new A();
  }
}

//// c.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module C;

public function test(): void {
  $x = new B();
  $y = $x->getA();
  // TODO: This should fail, coming soon...
  $y->foo();
}
