<?hh

class A  {
  const c = 1;
  public static $f1 = "42";
}

function f() {
  $a = A::c; // no errors
  $b = A::$f1; // no errors
  $n = "f";
  $c = A::${$n . "1"}; // error in strict mode
}
