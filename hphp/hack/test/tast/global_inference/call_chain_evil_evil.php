//// file1.php
<?hh //partial

class A {
  public static function f($x) {
    return $x;
  }
}

//// file2.php
<?hh //partial

class B extends A {
  public static function f($x) {
    return A::f($x);
  }
}
//// file3.php
<?hh //partial

class C extends B {
  public static function f($x) {
    return B::f(A::f($x));
  }
}

//// file4.php
<?hh // partial

function f() {
  return C::f(B::f(A::f("")));
}
