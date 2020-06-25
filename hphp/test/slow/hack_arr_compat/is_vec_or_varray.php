<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function check(mixed $arg, string $descr) {
  printf(
    "is_vec_or_varray(%s) = %s\n",
    $descr,
    HH\is_vec_or_varray($arg) ? 'true' : 'false',
  );
}

<<__EntryPoint>>
function main(): void {
  check(null, 'null');
  check("abc", '"abc"');

  check(varray[1, 2, 3],  'varray[1, 2, 3]');
  check(vec[1, 2, 3],  'vec[1, 2, 3]');

  check(darray["a" => "a"], 'darray["a" => "a"]');
  check(dict["a" => "a"], 'dict["a" => "a"]');

  check(keyset[1, 2, 3], 'keyset[1, 2, 3]');

  check(Vector{1, 2, 3}, 'Vector{1, 2, 3}');
  check(Map{"a" => "a"}, 'Map{"a" => "a"}');
}
