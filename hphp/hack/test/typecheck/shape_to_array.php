<?hh //strict

type my_shape = shape(
  'x' => int,
  ...
);

class C {
  const int KEY1 = 1;
  const int KEY2 = 2;
}

function expect<T>(T $x):void { }

function test(my_shape $s): void {
  expect<darray<arraykey,mixed>>(Shapes::toArray($s));
  expect<array<int,bool>>(Shapes::toArray(shape()));
  expect<array<string,arraykey>>(Shapes::toArray(shape('x' => 4, 'y' => 'aaa')));
  expect<array<int,arraykey>>(Shapes::toArray(shape(C::KEY1 => 4, C::KEY2 => 'aaa')));
  $s = shape();
  $s['x'] = 3;
  expect<array<string,int>>(Shapes::toArray($s));
}
