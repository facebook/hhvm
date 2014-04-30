<?hh

// Trigger COW

function main() {

  // $cb is a lambda that expects a Map and mutates it in some way. That
  // should trigger COW, which we'll be able to observe by dumping both
  // $m and $im.
  $test = function($op, $cb) {
    echo "------ " . $op . " -------\n";
    $m = Map {0 => 1, 1 => 2, 2 => 3};
    $im = $m->toImmMap();
    $cb($m);
    var_dump($m, $im);
  };


  $test("ArraySet", function ($m) { $m[0] = 42; });
  $test("ArrayAppend", function ($m) { $m[] = Pair {3, 42}; });
  $test("add", function ($m) { $m->add(Pair {3, 42}); });
  $test("addAll", function ($m) { $m->addAll(Map {0 => Pair {3, 42}}); });
  $test("clear", function ($m) { $m->clear(); });
  $test("set", function ($m) { $m->set(0, 42); });
  $test("setAll", function ($m) { $m->setAll(Map {0 => 42}); });
  $test("removeKey", function ($m) { $m->removeKey(0); });
}

main();
