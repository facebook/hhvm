<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// A reduced version of Exception::__toString(). The type of $elem before the
// while loop is Obj<=C, but then it changes to Obj<=C | InitNull inside the
// loop.

class C {
  private ?C $next = null;

  public function iter() {
    $elem = $this;

    while ($elem !== null) {
      $elem = $elem->next;
    }
  }
}

function main() {
  $c = new C();

  $c->iter();
  $c->iter();
  $c->iter();
}

main();
