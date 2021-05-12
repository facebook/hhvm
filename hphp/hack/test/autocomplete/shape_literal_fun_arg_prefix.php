<?hh

function takes_shape(shape('foo' => int, 'foobar' => float, 'goo' => string, ...) $_): void {}

function demo(): void {
  takes_shape(shape('fAUTO332'));
}
