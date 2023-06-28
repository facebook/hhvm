<?hh

function foo1() :mixed{
  $x = 2;
  return $x === "2";
}

function foo2() :mixed{
  $x = 2;
  return $x !== "2";
}

function foo3() :mixed{
  $x = "2";
  return $x === 2;
}

function foo4() :mixed{
  $x = "2";
  return $x !== 2;
}

function foo5() :mixed{
  $x = 2;
  return HH\Lib\Legacy_FIXME\eq($x, "2");
}

function foo6() :mixed{
  $x = 2;
  return HH\Lib\Legacy_FIXME\neq($x, "2");
}

function foo7() :mixed{
  $x = "2";
  return HH\Lib\Legacy_FIXME\eq($x, 2);
}

function foo8() :mixed{
  $x = "2";
  return HH\Lib\Legacy_FIXME\neq($x, 2);
}

function foo9($x) :mixed{
  $y = $x + 1;
  return $y === 6;
}

function foo10($x) :mixed{
  $y = $x + 1;
  return $y !== 6;
}


function foo12($x) :mixed{
  return $x !== 6;
}
<<__EntryPoint>> function main(): void {
var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6());
var_dump(foo7());
var_dump(foo8());
var_dump(foo9(5));
var_dump(foo10(5));
var_dump(foo12("6"));
}
