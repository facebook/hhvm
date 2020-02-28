<?hh

function f() {
  $v = Vector {
'a', 'b', 'c'}
;
  $m = new Map($v);
  var_dump($m);
}
function g() {
  $m = Map {
'a' => 1, 2 => 'b'}
;
  $v = new Vector($m);
  var_dump($v);
}
function h() {
  $arr1 = varray[11, 22, 33];
  var_dump(new Vector($arr1));
  var_dump(new Map($arr1));
  $arr2 = darray['a' => 1, 2 => 'b'];
  var_dump(new Vector($arr2));
  var_dump(new Map($arr2));
}
function gen() {
  yield 42;
  yield 72;
}
function j() {
  $v = new Vector(gen());
  var_dump($v);
}

<<__EntryPoint>>
function main_822() {
f();
g();
h();
j();
}
