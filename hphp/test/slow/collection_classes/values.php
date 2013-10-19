<?hh
function main() {
  $fn = function($x) { return $x; };
  $x = Vector {'abc'};
  var_dump($x->values());
  var_dump(get_class($x->lazy()->values()));
  var_dump(get_class($x->lazy()->map($fn)->values()));
  $x = Map {123 => 'abc'};
  var_dump($x->values());
  var_dump(get_class($x->lazy()->values()));
  var_dump(get_class($x->lazy()->map($fn)->values()));
  $x = StableMap {123 => 'abc'};
  var_dump($x->values());
  var_dump(get_class($x->lazy()->values()));
  var_dump(get_class($x->lazy()->map($fn)->values()));
  $x = Set {'abc'};
  var_dump($x->values());
  var_dump(get_class($x->lazy()->values()));
  var_dump(get_class($x->lazy()->map($fn)->values()));
}
main();

