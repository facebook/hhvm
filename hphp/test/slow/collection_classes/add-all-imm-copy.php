<?hh

function testSet(Set $a, Set $b) {
  var_dump($a->toImmSet());
  $a->addAll($b);
  var_dump($a->toImmSet());
}


<<__EntryPoint>>
function main_add_all_imm_copy() {
testSet(Set { }, Set { 'a' });
}
