<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function examine(mixed $val) :mixed{
  if (is_varray($val)) {
    echo "varray ";
  } else if (is_darray($val)) {
    echo "darray ";
  } else if (is_array($val)) {
    echo "legacy array ";
  }
  var_dump($val);
}

function test_unserialize($options) :mixed{
  examine(unserialize('a:0:{}', $options));
  examine(unserialize('a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}', $options));
  examine(unserialize('y:0:{}', $options));
  examine(unserialize('y:3:{i:1;i:2;i:3;}', $options));
  examine(unserialize('Y:0:{}', $options));
  examine(unserialize('Y:2:{i:1;i:1;i:2;i:2;}', $options));
}

<<__EntryPoint>>
function main() :mixed{
  echo "=== serialize ===\n";
  $options = dict['keepDVArrays' => true];
  var_dump(HH\serialize_with_options(vec[], $options));
  var_dump(HH\serialize_with_options(vec[1, 2, 3], $options));
  var_dump(HH\serialize_with_options(dict[], $options));
  var_dump(HH\serialize_with_options(dict[1 => 1, 2 => 2], $options));
  var_dump(HH\serialize_with_options(vec[], $options));
  var_dump(HH\serialize_with_options(vec[1, 2, 3], $options));
  var_dump(HH\serialize_with_options(dict[], $options));
  var_dump(HH\serialize_with_options(dict[1 => 1, 2 => 2], $options));

  echo "=== unserialize ===\n";
  test_unserialize(dict[]);

  echo "=== unserialize (force_darrays) ===\n";
  test_unserialize(dict['force_darrays' => true]);

}
