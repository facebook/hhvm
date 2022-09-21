//// b.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module B {
  imports {
    A
  }
  exports {
    C
  }
}

//// a.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
function f1(): C1 {
  return new C1();
}

class C1 {}

//// b.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module B;

public function f2(): C1 {
  return f1();
}

//// main.php
<?hh

function f3(): C1 {
  return f2();
}

function main(): void {
  $x = f3();
}
