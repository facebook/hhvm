<?hh

function f() :mixed{
  $v = Vector {
'a', 'b', 'c'}
;
  $m = new Map($v);
  var_dump($m);
}
function g() :mixed{
  $m = Map {
'a' => 1, 2 => 'b'}
;
  $v = new Vector($m);
  var_dump($v);
}
function h() :mixed{
  $arr1 = vec[11, 22, 33];
  var_dump(new Vector($arr1));
  var_dump(new Map($arr1));
  $arr2 = dict['a' => 1, 2 => 'b'];
  var_dump(new Vector($arr2));
  var_dump(new Map($arr2));
}
function gen() :AsyncGenerator<mixed,mixed,void>{
  yield 42;
  yield 72;
}
function j() :mixed{
  $v = new Vector(gen());
  var_dump($v);
}

<<__EntryPoint>>
function main_822() :mixed{
f();
g();
h();
j();
}
