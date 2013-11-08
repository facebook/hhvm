<?php

abstract class one {
  abstract protected function foo();
}

class a extends one {
  protected function foo() { echo "a\n"; }
}

class b extends one {
  protected function foo() { echo "b\n"; }
}

class c extends one {
  protected function foo() { echo "c\n"; }

  public function go($x) {
    $x->foo();
  }
}

function main() {
  $a = new a;
  $b = new b;
  $c = new c;
  $c->go($a);  // fill
  $c->go($a);  // hit
  $c->go($b);  // would call with not AttrPublic
  $c->go($c);  // again
  $c->go($c);  // hit
  $c->go($a);  // would call, not attr public
}

main();
