<?hh
class C {
  public $t = Pair {
'foo', 42}
;
}
function f() {
  $c = new C;
  $t = $c->t;
  var_dump(count($t));
  var_dump($t->count());
  var_dump($t->isEmpty());
  echo "------------------------\n";
  foreach ($t as $k => $v) {
    var_dump($k, $v);
  }
  echo "------------------------\n";
  var_dump($t[0], $t[1]);
  echo "------------------------\n";
  var_dump($t->at(0), $t->at(1));
  echo "------------------------\n";
  var_dump($t->get(0), $t->get(1), $t->get(2));
  echo "------------------------\n";
  var_dump((array)$t);
  echo "------------------------\n";
  var_dump(serialize($t));
  var_dump(unserialize(serialize($t)));
  echo "------------------------\n";
  var_dump($t->count());
  echo "------------------------\n";
  var_dump($t->getIterator() instanceof Iterator);
  var_dump($t->getIterator() instanceof KeyedIterator);
  foreach ($t->getIterator() as $k => $v) {
    var_dump($k, $v);
  }
  echo "------------------------\n";
  var_dump((array)$t);
  var_dump($t->toArray());
  echo "------------------------\n";
  var_dump(clone $t);
}
f();
