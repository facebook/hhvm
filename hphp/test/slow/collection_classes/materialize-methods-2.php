<?hh
function main() {
  $x = Map {123 => 'abc'};
  var_dump($x->lazy()->toVector());
  var_dump($x->lazy()->toMap());
  var_dump($x->lazy()->toStableMap());
  var_dump($x->lazy()->toSet());
  $fn = function($x) { return $x; };
  var_dump($x->lazy()->map($fn)->toVector());
  var_dump($x->lazy()->map($fn)->toMap());
  var_dump($x->lazy()->map($fn)->toStableMap());
  var_dump($x->lazy()->map($fn)->toSet());
}
main();
