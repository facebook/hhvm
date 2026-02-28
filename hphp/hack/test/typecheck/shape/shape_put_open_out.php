<?hh

function test(shape() $shape): shape(...) {
  return Shapes::put($shape, 'x', 'test');
}
