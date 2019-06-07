<?hh

class A {
  function f() {
    $a = function() {
 yield 1;
 yield 2;
 }
;
    return $a;
  }
}

<<__EntryPoint>>
function main_2155() {
$a = new A;
$f = $a->f();
foreach ($f() as $v) {
 var_dump($v);
 }
}
