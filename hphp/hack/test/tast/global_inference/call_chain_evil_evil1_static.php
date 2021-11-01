//// file1.php
<?hh

class A {
  public static function f($x) {
    return $x;
  }
}

//// file2.php
<?hh

class B extends A {
  public static function f($x) {
    return A::f($x);
  }
}

//// file3.php
<?hh

function f() {
  return B::f(A::f(""));
}
