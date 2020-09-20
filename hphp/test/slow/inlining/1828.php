<?hh

function id($x) {
 return $x;
 }
class X {
  public function f() {
 return 'hello';
 }
}
function test($a, $b) {
  return $a ? $b : id(new X)->f();
}

<<__EntryPoint>>
function main_1828() {
var_dump(test(false, false));
}
