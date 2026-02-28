<?hh

function test(): void {
  $x = shape('foo' => 1, 'bar' => true, 'xyz' => 3.14, 'baz' => 'abc');
  $y = shape('bar' => true, 'baz' => 'abc', 'foo' => 1, 'xyz' => 3.14);
  hh_show($x);
  hh_show($y);
}
