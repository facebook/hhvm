//// module_a.php
<?hh
new module a {}     // package pkg1
//// module_b.php
<?hh
new module b.b1 {}  // package pkg2

//// a.php
<?hh

module a;

interface IA {
  <<__CrossPackage("pkg2")>>
  public function test2(): void;
}

class A implements IA {
  <<__CrossPackage("pkg2")>>
  public function test() : void {
  }
  <<__CrossPackage("pkg1")>> // error cross package mismatch
  public function test2(): void {
  }
}
class B extends A implements IA  {
  <<__Override>>
  public function test(): void {} // ok
}
class C extends B implements IA  {
  <<__Override, __CrossPackage("pkg2")>>
  public function test(): void {} // error
}

class E implements IA {
  <<__CrossPackage("pkg2")>> // ok
  public function test2(): void {
  }
}

class F implements IA  {
  public function test2(): void { // ok
  }
}

//// b.php
<?hh

module b.b1;
// package pkg2 includes pkg1, so this is okay
//
class D extends A {
  public function test(): void {}

}
