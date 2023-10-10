<?hh

function bar($_1, $_2, inout $_3) :mixed{
  var_dump(__METHOD__);
  return shape('value' => null);
}

<<__EntryPoint>>
function main(): void {
  $s = shape('x' => 4);

  var_dump(Shapes::idx($s, 'x'));
  var_dump(Shapes::idx($s, 'x', 42));
  var_dump(Shapes::idx($s, 'y'));
  var_dump(Shapes::idx($s, 'y', 42));

  fb_intercept2('HH\Shapes::idx', 'bar');
}
