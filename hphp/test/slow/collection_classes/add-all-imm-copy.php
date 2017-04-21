<?hh

function testSet(Set $a, Set $b) {
  var_dump($a->toImmSet());
  $a->addAll($b);
  var_dump($a->toImmSet());
}

testSet(Set { }, Set { 'a' });
