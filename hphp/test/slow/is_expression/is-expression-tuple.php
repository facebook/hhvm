<?hh

function is_tuple(mixed $x): void {
  if ($x is (int, ?bool, string)) {
    echo "tuple\n";
  } else {
    echo "not tuple\n";
  }
}


<<__EntryPoint>>
function main() {
  is_tuple(null);
  is_tuple(new stdClass());
  is_tuple(darray[
    'one' => 2,
    'false' => false,
    'string' => 'string',
  ]);
  is_tuple(darray[
    0 => 2,
    1 => false,
    2 => 'string',
  ]); // TODO(T29967020)
  is_tuple(varray[]);
  is_tuple(tuple(1, false));
  is_tuple(tuple(1, 'string'));
  is_tuple(tuple(1, null, 1.5));
  is_tuple(tuple(1, null, 'string'));
  is_tuple(varray[1, null, 'string']);
  is_tuple(vec[1, null, 'string']);
  is_tuple(varray[1, null, 'string']);
}
