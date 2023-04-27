//// a.module.php
<?hh


new module A {}

//// b.module.php
<?hh


new module B {}

//// a.php
<?hh

module A;

public function f1(): C1 {
  return new C1();
}

public class C1 {}

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
