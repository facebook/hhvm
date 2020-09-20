<?hh

function main() {
  $v = Vector {};
  var_dump($v->map($x ==> $x + 1));
}

<<__EntryPoint>>
function main_vector_map_empty() {
main();
}
