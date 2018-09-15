<?hh

class MyWrapper<T> {}

function take_shape(MyWrapper<shape('x' => ?int)> $x): void {}

function test(MyWrapper<shape('x' => int)> $x): void {
  take_shape($x);
}
