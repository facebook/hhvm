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

class C extends B {
  public static function f($x) {
    return B::f(A::f($x));
  }
}

//// file4.php
<?hh

function f() {
  return C::f(B::f(A::f("")));
}
