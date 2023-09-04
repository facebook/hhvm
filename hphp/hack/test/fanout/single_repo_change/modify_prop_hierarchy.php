//// base-a.php
<?hh

class A {
  public int $foo = 0;
}

//// base-b.php
<?hh

class B extends A {
  public int $foo = 0;
}

//// base-c.php
<?hh

class C extends B {}

//// base-d.php
<?hh

class D extends C {
  public int $foo = 0;
}

//// base-e.php
<?hh

class E extends C {}

//// base-f.php
<?hh

class F extends E {
  public int $foo = 0;
}

//// base-g.php
<?hh

class G extends F {}

//// base-use-b.php
<?hh

function f_b(B $x): void {
  $x->foo = 0;
}

//// base-use-c.php
<?hh

function f_c(C $x): void {
  $x->foo = 0;
}

//// base-use-d.php
<?hh

function f_d(D $x): void {
  $x->foo = 0;
}
//// base-use-g.php
<?hh

function f_g(G $x): void {
  $x->foo = 0;
}

//// changed-a.php
<?hh

class A {
  public int $foo = 0;
}

//// changed-b.php
<?hh

class B extends A {
  public string $foo = 0;
}

//// changed-c.php
<?hh

class C extends B {}

//// changed-d.php
<?hh

class D extends C {
  public int $foo = 0;
}

//// changed-e.php
<?hh

class E extends C {}

//// changed-f.php
<?hh

class F extends E {
  public int $foo = 0;
}

//// changed-g.php
<?hh

class G extends F {}

//// changed-use-b.php
<?hh

function f_b(B $x): void {
  $x->foo = 0;
}

//// changed-use-c.php
<?hh

function f_c(C $x): void {
  $x->foo = 0;
}

//// changed-use-d.php
<?hh

function f_d(D $x): void {
  $x->foo = 0;
}
//// changed-use-g.php
<?hh

function f_g(G $x): void {
  $x->foo = 0;
}
