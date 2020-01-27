//// file1.php
<?hh //partial

class A {
  public function f($x) {
    return $x;
  }
}

//// file2.php
<?hh //partial

class B extends A {
  public function f($x) {
    return parent::f($x);
  }
}

//// file3.php
<?hh // partial

function f(A $a, B $b) {
  return $b->f($a->f(""));
}
