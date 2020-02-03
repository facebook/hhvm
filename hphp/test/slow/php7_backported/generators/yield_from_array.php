<?hh
function from() {
  yield 0;
  yield from varray[]; // must not yield anything
  yield from varray[1,2];
}
function gen() {
  yield from from();
}

<<__EntryPoint>>
function main_yield_from_array() {
foreach(gen() as $v) {
  var_dump($v);
}
}
