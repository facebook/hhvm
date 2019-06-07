<?hh

class X {
 function foo($a) {
 echo 'In foo:';
 var_dump($a);
 }
 }
function y($y) {
 echo 'In y:';
 var_dump($y);
 }
function test($x, $y) {
  $x->foo($x = null);
  $y($y = null);
}

<<__EntryPoint>>
function main_1526() {
test(new X, 'y');
}
