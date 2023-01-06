//// b.module.php
<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module B {
  imports {
    global
  }
  exports {
    global
  }
}

//// a.php
<?hh

function f1(): C1 {
  return new C1();
}

class C1 {}

//// b.php
<?hh


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
