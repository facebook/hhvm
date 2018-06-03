<?hh

set_error_handler(($no, $string) ==> {
  throw new Exception($string);
});

enum KeyType: int as int {
  ZERO = 0;
  ONE = 1;
}

type Tshape = shape(
  KeyType::ZERO => int,
  KeyType::ONE => string,
);

function is_shape(mixed $x): void {
  if ($x is Tshape) {
    echo "shape\n";
  } else {
    echo "not shape\n";
  }
}

is_shape(darray[0 => 0, 1 => 'one']);
is_shape(varray[0, 'one']); // TODO(T29967020)
