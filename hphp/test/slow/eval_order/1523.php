<?hh

function f(inout $a, $v = 5) {
  $a = $v;
  return 0;
}



function g(inout $a) {
  $a[0] = 5;
  return 0;
}
function h(inout $a) {
  $a = 5;
  return 0;
}
function k($a) {
  $a->prop = 5;
  return 0;
}
function foo() {
  return 'foo';
}
function dump($a, $b) {
  var_dump($a, $b);
}

<<__EntryPoint>>
function main_1523() {
$a = 2;
var_dump($a . f(inout $a));
$a = 2;
var_dump(($a.'') . f(inout $a));
$a = 2;
var_dump(($a.$a) . f(inout $a));
f(inout $a,2);
var_dump($a . f(inout $a));
f(inout $a,2);
var_dump(($a.'') . f(inout $a));
f(inout $a,2);
var_dump(($a.$a) . f(inout $a));


$a = varray[2];
var_dump($a[0] . g(inout $a));




$a = varray[2];
var_dump(($a[0] . '') . g(inout $a));
$a = new stdClass;
$a->prop = 2;
var_dump($a->prop . k($a));
$a = new stdClass;
$a->prop = 2;
var_dump(($a->prop . '') . k($a));
$i = 0;
var_dump($i . ++$i);
$i = 0;
var_dump(($i . '') . ++$i);
f(inout $a, 'test');
var_dump(($a . 'x') . foo($a = ''));
$a = new stdClass;
$a->foo = 42;
var_dump($a->{
$a = 'foo'}
);
var_dump($a);
$b = new stdClass;
$a = null;
$a->{
f(inout $a,$b)}
 = 5;
var_dump($a, $b);
f(inout $a, 'foo');
dump($a, $a = 'bar');
$a = 'foo';
dump($a, $a = 'bar');
f(inout $a, 'foo');
dump($a.'', $a = 'bar');
f(inout $a, 'foo');
dump($a.$a, $a = 'bar');
}
