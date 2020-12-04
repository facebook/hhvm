<?hh

class D {
  public bool $data = true;
}

class C {
  <<__Policied("B")>> public bool $b = false;
  <<__Policied("D")>> public ?D $d = null;
}

<<__InferFlows>>
function f(C $c, Traversable<D> $t): void {
  foreach($t as $d) {
    // we force $t to contain
    // objects that are subject to
    // the policy D
    $c->d = $d;
  }

  foreach($t as $d) {
    // Bad flow from B to D
    $d->data = $c->b;
  }
}
