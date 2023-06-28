<?hh

function foo1(bool $a) :mixed{
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($a) + 1.1;
}

function foo2() :mixed{
  return NAN + NAN;
}

function foo3() :mixed{
  return NAN + 0.0;
}

function foo4() :mixed{
  return NAN + 1.1;
}

function foo5() :mixed{
  return NAN + INF;
}

function foo6() :mixed{
  return INF + INF;
}

function foo7() :mixed{
  return 6 + INF;
}
<<__EntryPoint>> function main(): void {
var_dump(foo1(true));
var_dump(foo2());
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6());
var_dump(foo7());
}
