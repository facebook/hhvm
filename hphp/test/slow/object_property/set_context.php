<?hh

class X {
  private $p;

  function g() { return $this->p; }
  function s(X $c) { $c->p = $this; }
}
class Y extends X {}

<<__EntryPoint>>
function main() {
  $y = new Y();
  $x = new X();
  $x->s($y);

  if ($y->g() is Y) {
    echo "FAILED\n";
  } else {
    echo "OK\n";
  }
}
