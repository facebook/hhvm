<?hh
function dump($x) {
  var_dump(get_class($x));
  var_dump((array)$x);
}
function dump_unordered($x) {
  var_dump(get_class($x));
  $arr = (array)$x;
  ksort(&$arr, SORT_STRING);
  var_dump($arr);
  sort(&$arr, SORT_STRING);
  var_dump($arr);
}
function main() {
  $x = Vector {
    Vector {'a', 2},
    Map {'A' => 'a', 1 => 2},
    Pair {'a', 2},
  };
  foreach ($x as $v) {
    dump($v->toVector());
    dump($v->toMap());
    dump($v->toSet());
  }
  $x = Vector {
    Map {'A' => 'a', 1 => 2},
    Set {'a', 2},
  };
  foreach ($x as $v) {
    dump_unordered($v->toVector());
    if (!($v instanceof Set)) {
      dump_unordered($v->toMap());
    }
    dump_unordered($v->toSet());
  }
}

<<__EntryPoint>>
function main_materialize_methods_1() {
main();
}
