<?hh

function vec_append(): void {
  $v = vec[dict['a' => 42]];
  $v[] = dict['b' => 42.0];
  inspect($v);
}

function vector_append(): void {
  $v = Vector {dict['a' => 42]};
  $v[] = dict['b' => 42.0];
  inspect($v);
}
