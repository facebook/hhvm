<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function check(mixed $arg, string $descr) :mixed{
  printf(
    "is_vec_or_varray(%s) = %s\n",
    $descr,
    HH\is_vec_or_varray($arg) ? 'true' : 'false',
  );


}

<<__EntryPoint>>
function main(): void {
  $inputs = vec[
    tuple(null, 'null'),
    tuple("abc", '"abc"'),

    tuple(vec[1, 2, 3],  'vec[1, 2, 3]'),

    tuple(dict["a" => "a"], 'dict["a" => "a"]'),

    tuple(keyset[1, 2, 3], 'keyset[1, 2, 3]'),

    tuple(Vector{1, 2, 3}, 'Vector{1, 2, 3}'),
    tuple(Map{"a" => "a"}, 'Map{"a" => "a"}'),
  ];

  echo "=== constant values ===\n";
  foreach ($inputs as list($arg, $descr)) {
    check($arg, $descr);
  }

  echo "=== laundered values ===\n";
  foreach ($inputs as list($arg, $descr)) {
    check(__hhvm_intrinsics\launder_value($arg), $descr);
  }
}
