<?hh

class X {
  function foo() :mixed{
 var_dump(__METHOD__);
 }
  }
function foo($a) :mixed{
  $x = null;
  if ($a) $x = new X;
  return $x;
}
function test($x, $a) :mixed{
  $x = foo($a);
  if ($x) {
    $x->foo();
  }
}

<<__EntryPoint>>
function main_671() :mixed{
test(45, 0);
test(77, 1);
}
