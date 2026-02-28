<?hh
function foo1() :mixed{
  $x = 3;
  $y = 4;
  return $x * $y;
}

function foo2($x) :mixed{
  return $x * 2;
}

function foo3($x, $y) :mixed{
  return $x * $y;
}

function foo4($x) :mixed{
  return (-1) * $x;
}

function foo5($x, $y, $z) :mixed{
  return $x * $y + $x * $z;
}

function foo6($x, $y, $z) :mixed{
  return $y * $x + $x * $z;
}

function foo7($x, $y, $z) :mixed{
  return $x * $y + $z * $x;
}

function foo8($x, $y, $z) :mixed{
  return $y * $x + $z * $x;
}

function foo9($x, $y) :mixed{
  return ($x * 3) * ($y * 7);
}

function foo10($x) :mixed{
  return (3 * $x) * 7;
}

function foo11($x) :mixed{
  return $x * 8;
}

function foo12($x) :mixed{
  return $x * (-8);
}
<<__EntryPoint>> function main(): void {
var_dump(foo1());
var_dump(foo2(6));
var_dump(foo3(6,2));
var_dump(foo4(-12));
var_dump(foo5(2,1,5));
var_dump(foo6(2,1,5));
var_dump(foo7(2,1,5));
var_dump(foo8(2,1,5));
var_dump(foo9(2,5));
var_dump(foo10(2));
var_dump(foo11(7));
var_dump(foo12(3));
}
