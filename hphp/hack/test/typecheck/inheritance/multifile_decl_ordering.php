//// a.php
<?hh

class A {
  public function foo(): void {}
}

//// b.php
<?hh

class B extends A {}

//// b1.php
<?hh

class B1 extends A1 {}

//// a1.php
<?hh

class A1 {
  public function foo(): void {}
}
