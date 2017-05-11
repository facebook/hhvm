<?hh // strict

type MyShape = shape('x' => ?string);

function foo(array<MyShape> $shapes): void {
  $shapes[0]['x'] = 'some string';
}
