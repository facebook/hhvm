<?hh

type TTuple = (int, ?bool, string);

function is_tuple(mixed $x): void {
  if ($x is TTuple) {
    echo "tuple\n";
  } else {
    echo "not tuple\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_tuple() {
is_tuple(null);
is_tuple(new stdClass());
is_tuple(darray[
  'one' => 2,
  'false' => false,
  'string' => 'string',
]);
is_tuple(varray[]);
is_tuple(tuple(1, false));
is_tuple(tuple(1, 'string'));
is_tuple(tuple(1, null, 1.5));
is_tuple(tuple(1, null, 'string'));
is_tuple(varray[1, null, 'string']);
}
