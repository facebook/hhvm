//// base-a.php
<?hh

class A {
  public string $foo = "";
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

//// base-use.php
<?hh

function f(C $c): void {
  $c->foo = 0;
}

//// changed-a.php
<?hh

class A {
  public string $foo = 0;
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
  public int $foo = 0;
}

//// changed-use.php
<?hh

function f(C $c): void {
  $c->foo = 0;
}
