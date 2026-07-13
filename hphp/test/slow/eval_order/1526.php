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
  $recv = $x; $x = null; $recv->foo($x);
  $fn = $y; $y = null; $fn($y);
}

<<__EntryPoint>>
function main_1526() :mixed{
test(new X, y<>);
}
