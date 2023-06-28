<?hh

function foo1() :mixed{
  $x = 4;
  $y = 2;
  return $x ^ $y;
}


function foo3() :mixed{
  $x = 5;
  $y = 5;
  return $x ^ $y;
}

function foo4($x, $y) :mixed{
  return $x ^ $y;
}

function foo5($x) :mixed{
  return ~$x;
}

function foo6($x) :mixed{
  return !$x;
}
<<__EntryPoint>> function main(): void {
var_dump(foo1());
var_dump(foo3());
var_dump(foo4(3,5));
var_dump(foo5(1));
var_dump(foo6(5));
}
