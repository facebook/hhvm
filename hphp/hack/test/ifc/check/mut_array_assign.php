<?hh

class C {
  <<__Policied("I")>>
  public int $i = 0;
  <<__Policied("J")>>
  public int $j = 0;
  <<__Policied("PUBLIC")>>
  public Vector<int> $vector = Vector {};
}

<<__InferFlows>>
function ok(C $c): void {
  $vector = Vector {};
  $vector[] = $c->i;
  $c->i = $vector[0];
}

<<__InferFlows>>
function leak_via_value(C $c, Vector<int> $vector): void {
  $vector[] = $c->i;
  $c->j = $vector[0]; // I flows into J
}

<<__InferFlows>>
function leak_via_key(C $c, Vector<int> $vector): void {
  $vector[$c->i] = 0;
  $c->j = $vector[0]; // I flows into J
}

<<__InferFlows>>
function not_copy_on_write(C $c, Vector<int> $v): void {
  $v[] = $c->i; // I flows into PUBLIC
  $c->vector = $v;
  $v[] = $c->j; // J flows inot PUBLIC
}

<<__InferFlows>>
function leak_via_collection(C $c, Vector<shape()> $v0, Vector<shape()> $v1): void {
  if ($c->i) {
    $v = $v0;
  } else {
    $v = $v1;
  }
  $v[0] = shape();
  try {
    $v0[0];
    $c->j = 0; // I flows into J
    // If we $c->j is not zero we learn that $c->i was false.
  } catch (Exception $_) {
  }
}
