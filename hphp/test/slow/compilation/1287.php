<?hh

class X {
  function test($a, $b) {
    return $a != $b;
  }
}
function test($a) {
  $x = new X;
  return $a ? $x->test(1, 2) : false;
}

<<__EntryPoint>>
function main_1287() {
var_dump(test(1));
}
