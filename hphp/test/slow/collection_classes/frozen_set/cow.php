<?hh

// Trigger COW

function main() {

  // $cb is a lambda that expects a Set and mutates it in some way. That
  // should trigger COW, which we'll be able to observe by dumping both
  // $s and $is.
  $test = function($op, $cb) {
    echo "------ " . $op . " -------\n";
    $s = Set {1, 2, 3};
    $is = $s->toImmSet();
    $cb($s);
    var_dump($s, $is);
  };


  $test("add", function ($s) { $s->add(42); });
  $test("addAll", function ($s) { $s->addAll(Set {42}); });
  $test("addAllKeysOf", function ($s) { $s->addAllKeysOf(Map {42 => 'a'}); });
  $test("clear", function ($s) { $s->clear(); });
  $test("remove", function ($s) { $s->remove(1); });
}

main();
