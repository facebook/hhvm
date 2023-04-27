<?hh

function vec_access(): void {
  $v = vec[dict['a' => 42], dict['b' => 'apple']];
  $w = $v;
  $v[0] = dict['c' => 42.0];
  inspect($v[0]); // Expect 'c' to appear here
  inspect($w[0]); // But not here
}

function vector_access(): void {
  $v = Vector {dict['a' => 42], dict['b' => 'apple']};
  $w = $v;
  $v[0] = dict['c' => 42.0];
  inspect($v[0]); // Expect 'c' to appear here
  inspect($w[0]); // And here
}
