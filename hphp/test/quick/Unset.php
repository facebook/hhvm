<?hh

function f() {
  global $y;

  print ":".isset($x).":\n";
  print ":".isset($y).":\n";

  unset($x);
  unset($y);
  print ":".isset($x).":\n";
  print ":".isset($y).":\n";

  $x = 0;
  $y = 0;
  print ":".isset($x).":\n";
  print ":".isset($y).":\n";

  unset($x);
  unset($y);
  print ":".isset($x).":\n";
  print ":".isset($y).":\n";
}
f();

class A {
  public function foo() {
    unset($this);
  }
}
$obj = new A;
$obj->foo();
