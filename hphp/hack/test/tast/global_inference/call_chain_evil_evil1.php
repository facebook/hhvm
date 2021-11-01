//// file1.php
<?hh

class A {
  public function f($x) {
    return $x;
  }
}

//// file2.php
<?hh

class B extends A {
  public function f($x) {
    return parent::f($x);
  }
}

//// file3.php
<?hh

function f(A $a, B $b) {
  return $b->f($a->f(""));
}
