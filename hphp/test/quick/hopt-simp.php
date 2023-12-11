<?hh

function foo1() :mixed{
  $x = true;
  return !($x < false);
}

function foo2($x, $y) :mixed{
  return !($x < $y);
}

function foo3() :mixed{
  $x = 4;
  return 0 + $x;
}

function foo4() :mixed{
  $a = 5;
  $b = 2;
  return $a - $b;
}

function foo5() :mixed{
  $x = "ab";
  $y = "a";
  return (int)($x == $y);
}

function foo6($x) :mixed{
  return 2 + $x + 2;
}

function foo7($x) :mixed{
  $y = $x;
  return $x <= $y;
}

function foo8() :mixed{
  $x = vec[];
  return (bool)$x;
}

function foo9() :mixed{
  $x = vec[1, 2, 3];
  return (bool)$x;
}

<<__EntryPoint>> function main(): void {
var_dump(foo1());
var_dump(foo2(5, 6));
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6(2));
var_dump(foo7(2));
var_dump(foo8());
var_dump(foo9());
}
