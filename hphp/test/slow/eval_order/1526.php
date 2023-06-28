<?hh

class X {
 function foo($a) :mixed{
 echo 'In foo:';
 var_dump($a);
 }
 }
function y($y) :mixed{
 echo 'In y:';
 var_dump($y);
 }
function test($x, $y) :mixed{
  $x->foo($x = null);
  $y($y = null);
}

<<__EntryPoint>>
function main_1526() :mixed{
test(new X, y<>);
}
