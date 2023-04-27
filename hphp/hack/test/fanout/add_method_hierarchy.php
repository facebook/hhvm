//// base-a.php
<?hh

class A {
  public function foo(string $c): void {}
}

//// base-b.php
<?hh

class B extends A {}

//// base-c.php
<?hh

class C extends B {}

//// base-d.php
<?hh

class D extends C {
  public function foo(string $c): void {}
}

//// base-use.php
<?hh

function f(): void {
  $c->foo("");
}

//// changed-a.php
<?hh

class A {
  public function foo(string $c): void {}
}

//// changed-b.php
<?hh

class B extends A {
  public function foo(int $c): void {}
}

//// changed-c.php
<?hh

class C extends B {}

//// changed-d.php
<?hh

class D extends C {
  public function foo(string $c): void {}
}

//// changed-use.php
<?hh

function f(): void {
  $c->foo("");
}
