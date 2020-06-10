<?hh

function f($x, $y) {
  var_dump($x, $y);
  return $x + $x + $y + 1;
}


<<__EntryPoint>>
function main_531() {
var_dump(array_reduce(varray[], fun('f')));
var_dump(array_reduce(varray[], fun('f'), null));
var_dump(array_reduce(varray[], fun('f'), 0));
var_dump(array_reduce(varray[], fun('f'), 23));
var_dump(array_reduce(varray[4], fun('f')));
var_dump(array_reduce(varray[4], fun('f'), null));
var_dump(array_reduce(varray[4], fun('f'), 0));
var_dump(array_reduce(varray[4], fun('f'), 23));
var_dump(array_reduce(varray[1,2,3,4], fun('f')));
var_dump(array_reduce(varray[1,2,3,4], fun('f'), null));
var_dump(array_reduce(varray[1,2,3,4], fun('f'), 0));
var_dump(array_reduce(varray[1,2,3,4], fun('f'), 23));
}
