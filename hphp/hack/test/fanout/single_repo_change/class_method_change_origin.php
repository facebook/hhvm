//// base-a.php
<?hh
class A {
  public function foo(): void {}
}
//// base-b.php
<?hh
class B extends A {
  public function foo(): void {}
}
//// base-c.php
<?hh
class C extends B {}
//// base-use.php
<?hh
function use_a_foo(): void {
  (new A())->foo();
}
function use_b_foo(): void {
  (new B())->foo();
}
function use_c_foo(): void {
  (new C())->foo();
}

//// changed-a.php
<?hh
class A {
  // Position change here
  public function foo(): void {}
}
//// changed-b.php
<?hh
class B extends A {
  // Removing B::foo will change the origin field of C::foo and thus
  // trigger a type check of places that use C::foo! Weird!
}
//// changed-c.php
<?hh
class C extends B {}
//// changed-use.php
<?hh
function use_a_foo(): void {
  (new A())->foo();
}
function use_b_foo(): void {
  (new B())->foo();
}
function use_c_foo(): void {
  (new C())->foo();
}
