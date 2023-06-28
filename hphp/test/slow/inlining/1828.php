<?hh

function id($x) :mixed{
 return $x;
 }
class X {
  public function f() :mixed{
 return 'hello';
 }
}
function test($a, $b) :mixed{
  return $a ? $b : id(new X)->f();
}

<<__EntryPoint>>
function main_1828() :mixed{
var_dump(test(false, false));
}
