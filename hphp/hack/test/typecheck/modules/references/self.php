//// a.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    self.*
  }
}

//// a.x.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module A.X {
  imports {
    A.*
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

//// a.x.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module A.X;

internal function f2(): C1 {
  return f1();
}

//// main.php
<?hh

function main(): void {
}
