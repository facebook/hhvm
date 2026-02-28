<?hh

<<__SupportDynamicType>>
class C {
  const type TC = int;
}

function get_dyn():dynamic {
  return shape('a' => 3);
}

<<__SupportDynamicType>>
function test_key_exists(C::TC $_):void {
  $x = get_dyn();
  if (Shapes::keyExists($x, 'b')) {
    $y = $x['a'];
  }
  if (Shapes::keyExists($x, 'c')) {
    $y = $x['a'];
  }
}
