<?hh

function addAndDump($addMe) :mixed{
  $v = Vector {1, 2, 3};
  var_dump($v->immutable());
  $v->add($addMe);
  var_dump($v->immutable());
}


<<__EntryPoint>>
function main_vector_add_vs_immutable() :mixed{
addAndDump(4);
}
