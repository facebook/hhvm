<?hh
class C {
  public $t = Pair {'foo', 42};
}
function f() :mixed{
  $c = new C;
  $t = $c->t;
  $u = Pair {new stdClass, 73};
  var_dump(count($t), count($u));
  var_dump($t->count(), $u->count());
  var_dump($t->isEmpty(), $u->count());
  echo "------------------------\n";
  foreach ($t as $k => $v) {
    var_dump($k, $v);
  }
  echo "------------------------\n";
  foreach ($u as $k => $v) {
    var_dump($k, $v);
  }
  echo "------------------------\n";
  var_dump($t[0], $t[1]);
  var_dump($u[0], $u[1]);
  echo "------------------------\n";
  var_dump($t->at(0), $t->at(1));
  var_dump($u->at(0), $u->at(1));
  echo "------------------------\n";
  var_dump($t->get(0), $t->get(1), $t->get(2));
  var_dump($u->get(0), $u->get(1), $u->get(2));
  echo "------------------------\n";
  var_dump(darray($t), darray($u));
  echo "------------------------\n";
  var_dump(serialize($t));
  var_dump(serialize($u));
  var_dump(unserialize(serialize($t)));
  var_dump(unserialize(serialize($u)));
  echo "------------------------\n";
  var_dump($t->count(), $u->count());
  echo "------------------------\n";
  var_dump($t->getIterator() is Iterator);
  var_dump($u->getIterator() is Iterator);
  var_dump($t->getIterator() is KeyedIterator);
  var_dump($u->getIterator() is KeyedIterator);
  echo "------------------------\n";
  foreach ($t->getIterator() as $k => $v) {
    var_dump($k, $v);
  }
  echo "------------------------\n";
  foreach ($u->getIterator() as $k => $v) {
    var_dump($k, $v);
  }
  echo "------------------------\n";
  var_dump(darray($t), darray($u));
  var_dump($t->toVArray(), $u->toVArray());
  echo "------------------------\n";
  var_dump(clone $t, clone $u);
}

<<__EntryPoint>>
function main_823() :mixed{
f();
}
