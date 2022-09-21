//// a.x.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module A.X {
  exports {
    B.*
  }
}

//// b.y.module.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
<<file:__EnableUnstableFeatures('module_references')>>

new module B.Y {
  imports {
    A.*
  }
}

//// a.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module A.X;

public function f1(): C1 {
  return new C1();
}

public class C1 {}

//// b.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module B.Y;

internal function f2(): C1 {
  return f1();
}

//// main.php
<?hh

function main(): void {
}
