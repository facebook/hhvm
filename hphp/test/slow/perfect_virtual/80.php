<?hh

class X {
  function foo($a) {
 return $a;
 }
}
class Y extends X {
  function foo($a) {
 return $a;
 }
}
function test(X $x) {
  $y = $x->foo(5);
  return ++$y;
}

<<__EntryPoint>>
function main_80() {
test(new Y);
}
