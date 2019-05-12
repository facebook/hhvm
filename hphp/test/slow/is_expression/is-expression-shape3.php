<?hh

enum E: string {
  A = 'foo';
  B = 'bar';
}

function is_shape(mixed $x): void {
  if (
    $x is shape(
      E::A => int,
      ?E::B => int,
    )
  ) {
    echo "shape\n";
  } else {
    echo "not shape\n";
  }
}
<<__EntryPoint>> function main(): void {
is_shape(shape());
echo "\n";
is_shape(shape(E::A => 1));
is_shape(shape(E::A => 1, E::B => 1));
is_shape(shape(E::A => true));
is_shape(shape(E::A => 1, E::B => true));
echo "\n";
is_shape(shape('foo' => 1));
is_shape(shape('foo' => 1, 'bar' => 1));
is_shape(shape('foo' => true));
is_shape(shape('foo' => 1, 'bar' => true));
}
