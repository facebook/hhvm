<?hh

function addAndDump($addMe) :mixed{
  $v = Set {};
  var_dump($v->immutable());
  $v->addAll($addMe);
  var_dump($v->immutable());
}

function main() :mixed{
  addAndDump(vec[1, 2]);
  addAndDump(keyset[1, 2]);
  addAndDump(Set {1, 2});
}


<<__EntryPoint>>
function main_set_addall_vs_immutable() :mixed{
main();
}
