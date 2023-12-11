<?hh

function addAndDump($addMe) :mixed{
  $v = Vector {};
  var_dump($v->immutable());
  $v->addAll($addMe);
  var_dump($v->immutable());
}

function main() :mixed{
  addAndDump(vec[1, 2]);
  addAndDump(vec[1, 2]);
  addAndDump(Vector {1, 2});
}


<<__EntryPoint>>
function main_vector_addall_vs_immutable() :mixed{
main();
}
