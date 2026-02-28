<?hh

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
<<__EntryPoint>> function main(): void {
is_shape(dict[0 => 0, 1 => 'one']);
}
