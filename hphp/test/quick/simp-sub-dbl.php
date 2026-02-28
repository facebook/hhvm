<?hh

function foo1() :mixed{
  return NAN - NAN;
}

function foo2() :mixed{
  return 5.5 - NAN;
}

function foo3() :mixed{
  return NAN - 5.5;
}

function foo4($a) :mixed{
  return $a - 4.5;
}

function foo5() :mixed{
  return INF - 6.1;
}

function foo6() :mixed{
  return 4.4 - INF;
}

function foo7() :mixed{
  return INF - INF;
}

function foo8() :mixed{
  return NAN - INF;
}

function foo9() :mixed{
  return INF - NAN;
}

function foo10(float $a) :mixed{
  return $a - $a;
}
<<__EntryPoint>> function main(): void {
var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4(3.1));
var_dump(foo5());
var_dump(foo6());
var_dump(foo7());
var_dump(foo8());
var_dump(foo9());
var_dump(foo10(NAN));
}
