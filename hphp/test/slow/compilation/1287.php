<?hh

class X {
  function test($a, $b) :mixed{
    return $a != $b;
  }
}
function test($a) :mixed{
  $x = new X;
  return $a ? $x->test(1, 2) : false;
}

<<__EntryPoint>>
function main_1287() :mixed{
var_dump(test(1));
}
