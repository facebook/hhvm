//// base-file1.php
<?hh

class A {
  public function foo(): void {}
}

//// base-file2.php
<?hh

//// base-b.php
<?hh

class B extends A {
  <<__Override>>
  public function foo(): void {}
}

//// base-c.php
<?hh

class C extends B {}

//// base-f.php
<?hh

function f(B $x): B {
  return $x;
}

//// base-g.php
<?hh

function g(A $a): A {
  return $a;
}

//// base-h.php
<?hh

function h(C $c): void {
  $c->foo();
}

//// changed-file1.php
<?hh

//// changed-file2.php
<?hh

class A {
  public function foo(int $_): void {}
}

//// changed-b.php
<?hh

class B extends A {
  <<__Override>>
  public function foo(): void {}
}

//// changed-c.php
<?hh

class C extends B {}

//// changed-f.php
<?hh

function f(B $x): B {
  return $x;
}

//// changed-g.php
<?hh

function g(A $a): A {
  return $a;
}

//// changed-h.php
<?hh

function h(C $c): void {
  $c->foo();
}
