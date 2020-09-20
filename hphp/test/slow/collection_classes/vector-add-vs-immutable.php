<?hh

function addAndDump($addMe) {
  $v = Vector {1, 2, 3};
  var_dump($v->immutable());
  $v->add($addMe);
  var_dump($v->immutable());
}


<<__EntryPoint>>
function main_vector_add_vs_immutable() {
addAndDump(4);
}
