<?hh

class X {
  function __toString()[] :mixed{
 return 'hello';
 }
}
function f() :mixed{
  return 'bar';
}
function test() :mixed{
  $a = 'foo';
  for ($i = 0; $i < 10; $i++) {
    $a .= new X() . f();
  }
  return $a;
}

<<__EntryPoint>>
function main_783() :mixed{
var_dump(test());
}
