<?hh

interface I1 { const K = 1; }
interface I2 { const K = 2; }
trait T implements I2 {}
class X implements I1 {
  use T;
  function f() :mixed{ var_dump(X::K); }
}

function main() :mixed{
  (new X)->f();
}


<<__EntryPoint>>
function main_const_conflict() :mixed{
main();
}
