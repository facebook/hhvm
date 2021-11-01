<?hh

class A  {
  const int c = 1;
  static string $f1 = "42";
}

function f(): void {
  $a = A::c; // no errors
  $b = A::$f1; // no errors
}
