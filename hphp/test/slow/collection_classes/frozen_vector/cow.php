<?hh

// Trigger COW

function main() {

  // $cb is a lambda that expects a Vector and mutates it in some way.
  // That should trigger COW, which we'll be able to observe by dumping
  // both $v and $iv.
  $test = function($op, $cb) {
    echo "------ " . $op . " -------\n";
    $v = Vector {1, 2, 3};
    $iv = $v->toImmVector();
    $cb($v);
    var_dump($v, $iv);
  };


  $test("ArraySet", function ($v) { $v[0] = 42; });
  $test("ArrayAppend", function ($v) { $v[] = 42; });
  $test("add", function ($v) { $v->add(42); });
  $test("append", function ($v) { $v->append(42); });
  $test("addAll", function ($v) { $v->addAll(Vector {42}); });
  $test("addAllKeysOf",
        function ($v) { $v->addAllKeysOf(Map {42 => 'foo', 43 => 'bar'}); });
  $test("pop", function ($v) { $v->pop(); });
  $test("resize", function ($v) { $v->resize(0, 0); });
  $test("clear", function ($v) { $v->clear(); });
  $test("set", function ($v) { $v->set(0, 42); });
  $test("setAll", function ($v) { $v->setAll(Vector {42}); });
  $test("removeKey", function ($v) { $v->removeKey(0); });
  $test("reverse", function ($v) { $v->reverse(); });
  $test("splice", function ($v) { $v->splice(0, 1); });
}

main();
