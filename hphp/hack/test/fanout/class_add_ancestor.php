//// base-a.php
<?hh
class A {}
//// base-b.php
<?hh
class B {
  public function foo(): void {}
}
//// base-use-b.php
<?hh
function use_b_foo(): void {
  (new B())->foo();
}

//// changed-a.php
<?hh
class A {}
//// changed-b.php
<?hh
class B extends A {
  public function foo(): void {}
}
//// changed-use-b.php
<?hh
function use_b_foo(): void {
  (new B())->foo();
}
