<?hh

function f() :mixed{
 throw new Exception();
 }
function test() :mixed{
  $a = vec[1, f(), 2, f(), 3];
  var_dump($a);
}

<<__EntryPoint>>
function main_246() :mixed{
try {
 test();
 }
 catch (Exception $e) {
 }
}
