<?hh

function main($v) :mixed{
  $v[] = 4;
  $iv = $v->toImmVector();
  $v->reserve(1);
  var_dump($v->toImmVector());
}


<<__EntryPoint>>
function main_vector_reserve_vs_immutable() :mixed{
main(Vector {1, 2, 3});
}
