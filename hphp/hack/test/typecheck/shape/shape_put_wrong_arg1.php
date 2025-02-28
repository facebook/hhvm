<?hh

function test(shape() $shape): shape() {
  return Shapes::put(3, 'x', 'test');
}
