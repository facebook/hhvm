<?hh

class A {
  <<__Policied("PRIVATE")>>
  public dynamic $dynamic = 42;
  <<__Policied("PUBLIC")>>
  public dynamic $public = 42;
}

<<__InferFlows>>
function leak_implicitly(A $c): void {
  foreach ($c->dynamic as $k => $v) {
    $c->public = 0; // PRIVATE flows into PUBLIC
  }
}

<<__InferFlows>>
function ok(A $c): void {
  foreach ($c->dynamic as $k => $v) {
  }
  $c->public = 0;
}

<<__SupportDynamicType>>
class D {
  public bool $data = true;
}

<<__SupportDynamicType>>
class C {
  <<__Policied("B")>> public bool $b = false;
  <<__Policied("D")>> public ~?D $d = null;
}

<<__InferFlows>>
function f(C $c, dynamic $t): void {
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
