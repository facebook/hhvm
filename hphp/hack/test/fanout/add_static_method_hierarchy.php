//// base-a.php
<?hh

class A {
  public static function foo(string $c): void {}
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
  public static function foo(string $c): void {}
}

//// base-use.php
<?hh

function f(): void {
  C::foo("");
}

//// changed-a.php
<?hh

class A {
  public static function foo(string $c): void {}
}

//// changed-b.php
<?hh

class B extends A {
  public static function foo(int $c): void {}
}

//// changed-c.php
<?hh

class C extends B {}

//// changed-d.php
<?hh

class D extends C {
  public static function foo(string $c): void {}
}

//// changed-use.php
<?hh

function f(): void {
  C::foo("");
}
