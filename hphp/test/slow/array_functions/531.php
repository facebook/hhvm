<?hh

function f($x, $y) {
  var_dump($x, $y);
  return $x + $x + $y + 1;
}


<<__EntryPoint>>
function main_531() {
var_dump(array_reduce(varray[], 'f'));
var_dump(array_reduce(varray[], 'f', null));
var_dump(array_reduce(varray[], 'f', 0));
var_dump(array_reduce(varray[], 'f', 23));
var_dump(array_reduce(varray[4], 'f'));
var_dump(array_reduce(varray[4], 'f', null));
var_dump(array_reduce(varray[4], 'f', 0));
var_dump(array_reduce(varray[4], 'f', 23));
var_dump(array_reduce(varray[1,2,3,4], 'f'));
var_dump(array_reduce(varray[1,2,3,4], 'f', null));
var_dump(array_reduce(varray[1,2,3,4], 'f', 0));
var_dump(array_reduce(varray[1,2,3,4], 'f', 23));
}
