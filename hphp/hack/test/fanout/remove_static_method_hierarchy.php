//// base-a.php
<?hh

class A {
  public static function foo(string $c): void {}
}

//// base-b.php
<?hh

class B extends A {
  public static function foo(arraykey $c): void {}
}

//// base-c.php
<?hh

class C extends B {}

//// base-d.php
<?hh

class D extends C {
  public static function foo(nonnull $c): void {}
}

//// base-use.php
<?hh

function f(): void {
  C::foo(0);
}

//// changed-a.php
<?hh

class A {
  public static function foo(string $c): void {}
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
  public static function foo(nonnull $c): void {}
}

//// changed-use.php
<?hh

function f(): void {
  C::foo(0);
}
