<?hh

function test() {
  $y = 1;
  $foo = () ==> $x + $y;
  $x = 2;
  $r = new ReflectionFunction($foo);
  var_dump($r->isClosure()); // true
  var_dump($r->getStaticVariables()); // x, y
  $foo(); // x is not defined
}

test();
