<?hh

function test() {
  $v = new Vector;
  $v->reverse();
  var_dump($v);
}


<<__EntryPoint>>
function main_empty_vector() {
test();
}
