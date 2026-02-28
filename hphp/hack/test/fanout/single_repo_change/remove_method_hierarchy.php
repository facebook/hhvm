//// base-a.php
<?hh

class A {
  public function foo(string $c): void {}
}

//// base-b.php
<?hh

class B extends A {
  public function foo(int $c): void {}
}

//// base-c.php
<?hh

class C extends B {}

//// base-d.php
<?hh

class D extends C {
  public function foo(string $c): void {}
}

//// base-e.php
<?hh

class E extends C {}

//// base-f.php
<?hh

class F extends E {
  public function foo(string $c): void {}
}

//// base-g.php
<?hh

class G extends F {}

//// base-use-b.php
<?hh

function f_b(B $x): void {
  $x->foo("");
}

//// base-use-c.php
<?hh

function f_c(C $x): void {
  $x->foo("");
}

//// base-use-d.php
<?hh

function f_d(D $x): void {
  $x->foo("");
}
//// base-use-g.php
<?hh

function f_g(G $x): void {
  $x->foo("");
}

//// changed-a.php
<?hh

class A {
  public function foo(string $c): void {}
}

//// changed-b.php
<?hh

class B extends A {}

//// changed-c.php
<?hh

class C extends B {}

//// changed-d.php
<?hh

class D extends C {
  public function foo(string $c): void {}
}

//// changed-e.php
<?hh

class E extends C {}

//// changed-f.php
<?hh

class F extends E {
  public function foo(string $c): void {}
}

//// changed-g.php
<?hh

class G extends F {}

//// changed-use-b.php
<?hh

function f_b(B $x): void {
  $x->foo("");
}

//// changed-use-c.php
<?hh

function f_c(C $x): void {
  $x->foo("");
}

//// changed-use-d.php
<?hh

function f_d(D $x): void {
  $x->foo("");
}
//// changed-use-g.php
<?hh

function f_g(G $x): void {
  $x->foo("");
}
