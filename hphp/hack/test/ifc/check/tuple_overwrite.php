<?hh

class C {
  <<__Policied("PUBLIC")>>
  public int $public = 0;
  <<__Policied("PRIVATE")>>
  public int $private = 0;
}

<<__InferFlows>>
function overwrite_to_make_ok(C $c): void {
  $t = tuple($c->private, $c->public);
  $t[0] = $c->public;
  $c->public = $t[0];
}

<<__InferFlows>>
function overwrite_to_make_ko(C $c): void {
  $t = tuple($c->public, $c->public);
  $t[0] = $c->private;
  $c->public = $t[0]; // PRIVATE flows into PUBLIC
}

<<__InferFlows>>
function overwrite_to_stay_ok(C $c): void {
  $t = tuple($c->public, $c->public);
  $t[0] = $c->private;
  $c->public = $t[1];
}

<<__InferFlows>>
function overwrite_to_stay_ko(C $c): void {
  $t = tuple($c->private, $c->private);
  $t[0] = $c->public;
  $c->public = $t[1]; // PRIVATE flows into PUBLIC
}
