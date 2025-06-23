//// module_M1.php
<?hh
new module M1 {}

//// module_M2.php
<?hh
new module M2 {}

//// A.php
<?hh

module M1;

class A {
 protected internal static function foo(): void {}
 protected internal static function bar(): void {}
 internal static function baz(): void {}
 protected static function qux(): void {}
 public static function quux(): void {}
 protected static internal function corge(): void {}
 protected static internal int $foo = 42;
 protected static internal int $bar = 42;
 internal static int $baz = 42;
 protected static int $qux = 42;
 public static int $quux = 42;
 protected internal static int $corge = 42;
}

//// B.php
<?hh

class B extends A {
  // OK since we are widening visibility
  public static function foo(): void {}
  // OK since we are widening visibility
  protected static function bar(): void {}
  // Not OK since we are narrowing visibility
  private static function corge(): void {}
  // OK since we are widening visibility
  protected static int $foo = 42;
  // OK since we are widening visibility
  public static int $bar = 42;
}

//// C.php
<?hh

module M1;

class C extends A {
  // OK since we are in the same module
  protected internal static function foo(): void {}
  // OK since we are widening visibility and in the same module
  internal static function bar(): void {}
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal static function baz(): void {}
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal static function qux(): void {}
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal static function quux(): void {}
  // OK since we are in the same module
  protected internal static int $foo = 42;
  // OK since we are widening visibility and in the same module
  internal static int $bar = 42;
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal static int $baz = 42;
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal static int $qux = 42;
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal static int $quux = 42;
}

//// D.php
<?hh

module M2;

class D extends A {
  // Not OK since we are in a different module
  protected internal static function foo(): void {}
  // Not OK since we are in a different module
  internal static function bar(): void {}
  // Not OK since we are narrowing visibility and in a different module
  private static function corge(): void {}
  // Not OK since we are in a different module
  protected static internal int $foo = 42;
  // Not OK since we are in a different module
  internal static int $bar = 42;
  // Not OK since we are narrowing visibility and in a different module
  private static int $corge = 42;
}
