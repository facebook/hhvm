<?hh

type MyShape = shape('x' => ?string);

function foo(varray<MyShape> $shapes): void {
  $shapes[0]['x'] = 'some string';
}
