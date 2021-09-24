<?hh

function hgoldstein(
  shape(?'lol' => int) $s,
  dict<arraykey, mixed> $d,
  string $z,
): void {
  Shapes::idx(inout $s, 'lol');
  type_structure(inout $d, 'T');
  printf('%s', inout $z);
  is_null(inout $z);
  HH\is_dict_or_darray(inout $d);
  HH\is_any_array(inout $d);
  HH\is_vec_or_varray(inout $d);
  HH\is_dict_or_darray(inout $d);
}
