//// a.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module A {
  exports {
  }
}

//// b.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module B {
}

//// a.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A;

public trait A {
  public function foo(): void {
  }
}

//// b.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module B;

public class B {
  use A;
}

public function getB(): B {
  return new B();
}

//// main.php
<?hh

function test(): void {
  $x = getB();
  $x->foo();
}
