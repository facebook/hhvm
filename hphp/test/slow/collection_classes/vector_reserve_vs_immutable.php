<?hh

function main($v) {
  $v[] = 4;
  $iv = $v->toImmVector();
  $v->reserve(1);
  var_dump($v->toImmVector());
}


<<__EntryPoint>>
function main_vector_reserve_vs_immutable() {
main(Vector {1, 2, 3});
}
