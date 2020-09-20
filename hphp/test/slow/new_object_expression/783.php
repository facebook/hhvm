<?hh

class X {
  function __toString() {
 return 'hello';
 }
}
function f() {
  return 'bar';
}
function test() {
  $a = 'foo';
  for ($i = 0; $i < 10; $i++) {
    $a .= new X() . f();
  }
  return $a;
}

<<__EntryPoint>>
function main_783() {
var_dump(test());
}
