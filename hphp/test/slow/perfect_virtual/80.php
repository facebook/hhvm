<?hh

class X {
  function foo($a) :mixed{
 return $a;
 }
}
class Y extends X {
  function foo($a) :mixed{
 return $a;
 }
}
function test(X $x) :mixed{
  $y = $x->foo(5);
  return ++$y;
}

<<__EntryPoint>>
function main_80() :mixed{
test(new Y);
}
