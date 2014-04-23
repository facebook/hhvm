<?hh
function dump($x, $str = null) {
  echo ($str ?: get_class($x)) . "\n";
  var_dump($x->firstValue(), $x->firstKey(), $x->lastValue(), $x->lastKey());
}
function dump_set($x, $str = null) {
  echo ($str ?: get_class($x)) . "\n";
  var_dump($x->firstValue(), $x->lastValue());
}
function main() {
  dump(Vector {});
  dump(Vector {11, 22});
  dump(ImmVector {});
  dump(ImmVector {11, 22});
  dump(Map {});
  dump(Map {'a' => 11, 'b' => 22});
  dump(ImmMap {});
  dump(ImmMap {'a' => 11, 'b' => 22});
  dump_set(Set {});
  dump_set(Set {11, 22});
  dump_set(ImmSet {});
  dump_set(ImmSet {11, 22});
  dump(Pair {11, 22});
  dump((Map {'a' => 11, 'b' => 22})->lazy(), "Map lazy()");
  dump((Map {'a' => 11, 'b' => 22})->lazy()->map($v ==> $v),
       "Map lazy()->map()");
  dump_set((Set {11, 22})->lazy(), "Set lazy()");
  dump_set((Set {11, 22})->lazy()->map($v ==> $v), "Set lazy()->map()");
}
main();
