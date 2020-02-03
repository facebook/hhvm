<?hh

function gen() {
  yield from varray[1, 2, 3, 4];
  yield from varray[11, 12, 13];
  yield from varray[];
  yield from varray[31, 32, 33, 34, 35];
}


<<__EntryPoint>>
function main_yield_from_multiple_iterators() {
foreach(gen() as $val) {
  var_dump($val);
}
}
