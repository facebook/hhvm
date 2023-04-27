<?hh

function vec_test(): void {
  vec[dict['a' => 42], dict['b' => true]];
}

function keyset_test(): void {
  keyset['a', 'b']; // Tests that no errors are produced
}

function dict_test(): void {
  dict['oa_' => dict['a' => 42], 'ob' => dict['b' => true]];
}

function vector_test(): void {
  Vector {dict['a' => 42], dict['b' => true]};
}

function set_test(): void {
  Set {'a', 'b'}; // Tests that no errors are produced
}

function map_test(): void {
  Map {'oa' => dict['a' => 42], 'ob' => dict['b' => true]};
}
