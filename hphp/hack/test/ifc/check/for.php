<?hh

class C {
  <<__Policied("PRIVATE")>>
  public int $private = 0;
  <<__Policied("PUBLIC")>>
  public int $public = 0;
}

class D {
  public function __construct(public C $c) { }
}

function dont_leak_through_init(D $d): void {
  // Doesn't leak anything as initialisers are evaluated before the condition
  for ($d->c->public = 0; $d->c->private === 0;) { }
}

<<__InferFlows>>
function leak_through_incr(C $c): void {
  // Leaks the private context to public at the increment statement
  for (;$c->private === 0; $c->public++) { }
}

<<__InferFlows>>
function dont_leak_through_incr_continue(C $c): void {
  // Doesn't leak because we always continue
  for (;$c->private === 0;) {
    continue;
    $c->public++;
  }
}

<<__InferFlows>>
function leak_through_incr_continue(C $c): void {
  // Leaks the private context to public at the increment statement
  for (;$c->private === 0; $c->public++) {
    continue;
  }
}

<<__InferFlows>>
function forever(): int {
  for (;;) {
    return 10;
  }
}
