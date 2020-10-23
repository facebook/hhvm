<?hh

class C {
  <<__Policied("PUBLIC")>>
  public int $out = 0;
  <<__Policied("KEY")>>
  public int $key = 0;
  <<__Policied("VALUE")>>
  public int $value = 0;
}

<<__InferFlows>>
function leak_through_vec_value(C $c): void {
  $vec = vec[$c->value];
  foreach ($vec as $value) {
    // VALUE leaks into PUBLIC
    $c->out = $value;
  }
}

<<__InferFlows>>
function leak_through_dict_key_and_value(C $c): void {
  $dict = dict[$c->key => $c->value];
  foreach ($dict as $key => $value) {
    // KEY leaks into PUBLIC
    $c->out = $key;
    // VALUE leaks into PUBLIC
    $c->out = $value;
  }
}
