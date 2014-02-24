<?hh

// Trigger COW

function main() {

  // $cb is a lambda that expects a vector (reference) and mutates it
  // in some way. That should trigger COW, which we'll be able to
  // observe by var_dumping both $v and $fv.
  $mv = function($op, $cb) {
    echo "------ " . $op . " -------\n";
    $v = Vector {1, 2, 3};
    $fv = $v->toFrozenVector();
    $cb($v);
    var_dump($v, $fv);
  };


  $mv("ArraySet", function (&$v) { $v[0] = 42; });
  $mv("ArrayAppend", function (&$v) { $v[] = 42; });
  $mv("add", function (&$v) { $v->add(42); });
  $mv("append", function (&$v) { $v->append(42); });
  $mv("addAll", function (&$v) { $v->addAll(Vector {42}); });
  $mv("pop", function (&$v) { $v->pop(); });
  $mv("resize", function (&$v) { $v->resize(0, 0); });
  $mv("clear", function (&$v) { $v->clear(); });
  $mv("set", function (&$v) { $v->set(0, 42); });
  $mv("setAll", function (&$v) { $v->setAll(Vector {42}); });
  $mv("removeKey", function (&$v) { $v->removeKey(0); });
  $mv("reverse", function (&$v) { $v->reverse(); });
  $mv("splice", function (&$v) { $v->splice(0, 1); });
  //$mv("shuffle", function (&$v) { $v->shuffle(); });
}

main();
