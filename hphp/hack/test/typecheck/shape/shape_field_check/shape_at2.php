<?hh

type ClosedShape = shape(
  'required' => int,
);

function test(ClosedShape $closed_shape): void {
  $_ = Shapes::at($closed_shape, 'missing');
}
