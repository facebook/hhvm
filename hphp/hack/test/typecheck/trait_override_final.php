//// file1.php
<?hh

trait S {
  final public function f(): void {}
}

//// file2.php
<?hh

class B {
  final public function f(): void {}
}

//// file3.php
<?hh

class C extends B {
  use S;
}
