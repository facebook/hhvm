<?hh // partial

class A  {
  const c = 1;
  static $f1 = "42";
}

function f() {
  $a = A::c; // no errors
  $b = A::$f1; // no errors
}
