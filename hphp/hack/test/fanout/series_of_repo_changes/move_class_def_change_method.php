//// file1.php
<?hh

class A {
  public function foo(): void {}
}

//// file2.php
<?hh

//// b.php
<?hh

class B extends A {
  <<__Override>>
  public function foo(): void {}
}

//// c.php
<?hh

class C extends B {}

//// f.php
<?hh

function f(B $x): B {
  return $x;
}

//// g.php
<?hh

function g(A $a): A {
  return $a;
}

//// h.php
<?hh

function h(C $c): void {
  $c->foo();
}

///////////////

//// file1.php
<?hh

//// file2.php
<?hh

class A {
  public function foo(int $_): void {}
}
