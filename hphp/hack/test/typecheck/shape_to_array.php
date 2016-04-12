<?hh //strict

type my_shape = shape('x' => int);

class C {
  const int KEY1 = 1;
  const int KEY2 = 2;
}

function test(my_shape $s): void {
  hh_show(Shapes::toArray($s));
  hh_show(Shapes::toArray(shape()));
  hh_show(Shapes::toArray(shape('x' => 4, 'y' => 'aaa')));
  hh_show(Shapes::toArray(shape(C::KEY1 => 4, C::KEY2 => 'aaa')));
  $s = shape();
  $s['x'] = 3;
  hh_show(Shapes::toArray($s));
}
