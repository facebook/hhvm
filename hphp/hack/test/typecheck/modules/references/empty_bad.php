//// a.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module A {
  imports {
  }
  exports {
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

//// main.php
<?hh

function main(): void {
  $x = f1();
}
