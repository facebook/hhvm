<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function show_shape_type_optionals(
  shape(
    'a' => int,
    ?'b' => bool,
    ?'c' => ?float,
  ) $s,
): void {
  hh_show($s);
}
function show_shape_type_open(
  shape(
    'a' => int,
    ...
  ) $s,
): void {
  hh_show($s);
}
function show_shape_type_open_no_fields(shape(...) $s): void {
  hh_show($s);
}

type simple_shape = shape(
  'x' => int,
  ...
);
function show_shape_type_unset_fields(simple_shape $s): void {
  Shapes::removeKey(inout $s, 'b');
  hh_show($s);
}
