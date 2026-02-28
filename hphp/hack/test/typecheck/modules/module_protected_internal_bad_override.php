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
 protected internal function foo(): void {}
 protected internal function bar(): void {}
 internal function baz(): void {}
 protected function qux(): void {}
 public function quux(): void {}
 protected internal function corge(): void {}
 protected internal int $foo = 42;
 protected internal int $bar = 42;
 internal int $baz = 42;
 protected int $qux = 42;
 public int $quux = 42;
 protected internal int $corge = 42;
}

//// B.php
<?hh

class B extends A {
  // OK since we are widening visibility
  public function foo(): void {}
  // OK since we are widening visibility
  protected function bar(): void {}
  // Not OK since we are narrowing visibility
  private function corge(): void {}
  // OK since we are widening visibility
  protected int $foo = 42;
  // OK since we are widening visibility
  public int $bar = 42;
}

//// C.php
<?hh

module M1;

class C extends A {
  // OK since we are in the same module
  protected internal function foo(): void {}
  // OK since we are widening visibility and in the same module
  internal function bar(): void {}
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal function baz(): void {}
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal function qux(): void {}
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal function quux(): void {}
  // OK since we are in the same module
  protected internal int $foo = 42;
  // OK since we are widening visibility and in the same module
  internal int $bar = 42;
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal int $baz = 42;
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal int $qux = 42;
  // Not OK since we are narrowing visibility even though we are in the same module
  protected internal int $quux = 42;
}

//// D.php
<?hh

module M2;

class D extends A {
  // Not OK since we are in a different module
  protected internal function foo(): void {}
  // Not OK since we are in a different module
  internal function bar(): void {}
  // Not OK since we are narrowing visibility and in a different module
  private function corge(): void {}
  // Not OK since we are in a different module
  protected internal int $foo = 42;
  // Not OK since we are in a different module
  internal int $bar = 42;
  // Not OK since we are narrowing visibility and in a different module
  private int $corge = 42;
}
