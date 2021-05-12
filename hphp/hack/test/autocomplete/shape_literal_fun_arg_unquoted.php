<?hh

function takes_shape(shape('foo' => int, 'foobar' => float) $_): void {}

function demo(): void {
  takes_shape(shape(AUTO332));
}
