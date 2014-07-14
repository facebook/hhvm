<?hh
function test($x) {
  var_dump($x->toMap());
  var_dump($x->toImmMap());
  var_dump($x->lazy()->toMap());
  var_dump($x->lazy()->toImmMap());
  echo "----\n";
  var_dump($x->map($x ==> is_int($x) ? $x*2+1 : $x.'b'.$x)->toMap());
  var_dump($x->map($x ==> is_int($x) ? $x*2+1 : $x.'b'.$x)->toImmMap());
  var_dump($x->lazy()->map($x ==> is_int($x) ? $x*2+1 : $x.'b'.$x)->toMap());
  var_dump($x->lazy()->map($x ==> is_int($x) ? $x*2+1 : $x.'b'.$x)->toImmMap());
  echo "----\n";
  var_dump($x->keys());
  var_dump($x->lazy()->keys()->toVector());
  var_dump($x->firstKey(), $x->lastKey());
}
function main() {
  echo "==== Set ====\n";
  test(Set {5, 'a', 0, ''});
  echo "==== ImmSet ====\n";
  test(ImmSet {5, 'a', 0, ''});
}
main();
