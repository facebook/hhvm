<?hh

class X {
  const FOO = 1;
  const BAR = FIZ;
  const BAZ = FIZ;
  const BOO = FIZ;
  const BIZ = FIZ;
  const FIZ = FIZ;
}

function foo($a, $b) :mixed{
  var_dump($a, $b);
}

function f() :mixed{ return FIZ; }

function test() :mixed{
  foo(f(), vec[X::FOO, X::BAZ,
                 X::BAR, X::BAZ,
                 X::BOO, X::BIZ]);
}




const FIZ = 32;
<<__EntryPoint>>
function main_exitspill() :mixed{

test();
}
