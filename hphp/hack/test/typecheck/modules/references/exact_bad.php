//// a.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    C
  }
}

//// b.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module B {
  imports {
    C
  }
}

//// a.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module A;

public function f1(): C1 {
  return new C1();
}

public class C1 {}

//// b.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module B;

internal function f2(): C1 {
  return f1();
}

//// main.php
<?hh

function main(): void {
}
