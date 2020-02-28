<?hh

class X {
  const FOO = 1;
  const BAR = FIZ;
  const BAZ = FIZ;
  const BOO = FIZ;
  const BIZ = FIZ;
  const FIZ = FIZ;
}

function foo($a, $b) {
  var_dump($a, $b);
}

function f() { return FIZ; }

function test() {
  foo(f(), varray[X::FOO, X::BAZ,
                 X::BAR, X::BAZ,
                 X::BOO, X::BIZ]);
}




const FIZ = 32;
<<__EntryPoint>>
function main_exitspill() {

test();
}
