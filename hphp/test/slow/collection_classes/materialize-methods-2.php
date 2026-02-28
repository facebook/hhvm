<?hh
function main() :mixed{
  $x = Map {123 => 'abc'};
  var_dump($x->lazy()->toVector());
  var_dump($x->lazy()->toImmVector());
  var_dump($x->lazy()->toMap());
  var_dump($x->lazy()->toImmMap());
  var_dump($x->lazy()->toSet());
  var_dump($x->lazy()->toImmSet());
  $fn = function($x) { return $x; };
  var_dump($x->lazy()->map($fn)->toVector());
  var_dump($x->lazy()->map($fn)->toImmVector());
  var_dump($x->lazy()->map($fn)->toMap());
  var_dump($x->lazy()->map($fn)->toImmMap());
  var_dump($x->lazy()->map($fn)->toSet());
  var_dump($x->lazy()->map($fn)->toImmSet());
}

<<__EntryPoint>>
function main_materialize_methods_2() :mixed{
main();
}
