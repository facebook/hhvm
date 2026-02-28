<?hh

function identity<T>(T $x): T {
  return $x;
}

function expectVecInt(vec<int> $_):void { }
function expectBool(bool $_):void { }
function test1():void {
  $x = identity<_>(3);
  $y = identity<vec<_>>(vec[3]);
  expectVecInt($y);
  $z = identity<shape('a' => int, 'b' => _)>(shape('a' => 3, 'b' => false));
  expectBool($z['b']);
}
