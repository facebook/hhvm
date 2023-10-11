<?hh

type ClosedShape = shape(
  'required' => int,
  ?'optional' => string,
  'nullable' => ?float,
  ?'optional_nullable' => ?int,
);

type OpenShape = shape(...);

function test(ClosedShape $closed_shape, OpenShape $open_shape): void {
  hh_show(Shapes::at($closed_shape, 'required'));
  hh_show(Shapes::at($closed_shape, 'optional'));
  hh_show(Shapes::at($closed_shape, 'nullable'));
  hh_show(Shapes::at($closed_shape, 'optional_nullable'));
  hh_show(Shapes::at($open_shape, 'unknown'));
}
