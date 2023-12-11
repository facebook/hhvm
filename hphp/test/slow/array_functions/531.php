<?hh

function f($x, $y) :mixed{
  var_dump($x, $y);
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($x) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($x) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($y) + 1;
}


<<__EntryPoint>>
function main_531() :mixed{
var_dump(array_reduce(vec[], f<>));
var_dump(array_reduce(vec[], f<>, null));
var_dump(array_reduce(vec[], f<>, 0));
var_dump(array_reduce(vec[], f<>, 23));
var_dump(array_reduce(vec[4], f<>));
var_dump(array_reduce(vec[4], f<>, null));
var_dump(array_reduce(vec[4], f<>, 0));
var_dump(array_reduce(vec[4], f<>, 23));
var_dump(array_reduce(vec[1,2,3,4], f<>));
var_dump(array_reduce(vec[1,2,3,4], f<>, null));
var_dump(array_reduce(vec[1,2,3,4], f<>, 0));
var_dump(array_reduce(vec[1,2,3,4], f<>, 23));
}
