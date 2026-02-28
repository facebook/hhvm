<?hh

function f() :mixed{
  $a = function() {
 yield 1;
 yield 2;
 }
;
  return $a;
}

<<__EntryPoint>>
function main_2154() :mixed{
$f = f();
foreach ($f() as $v) {
 var_dump($v);
 }
}
