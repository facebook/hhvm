<?hh

function foo1() :mixed{
  return NAN * NAN;
}

function foo2() :mixed{
  return NAN * 2;
}

function foo3() :mixed{
  return INF * NAN;
}

function foo4() :mixed{
  return 2.0 * INF;
}

function foo5() :mixed{
  return INF * INF;
}

function foo6($a) :mixed{
  return $a * 2.0;
}
<<__EntryPoint>> function main(): void {
var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6(3.1));
}
