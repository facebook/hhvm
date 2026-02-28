<?hh

function bar($_1, $_2, inout $_3) :mixed{
  var_dump(__METHOD__);
  return shape('value' => null);
}

<<__EntryPoint>>
function main(): void {
  $vec = vec['red', 'green'];
  HH\Lib\C\contains($vec, 'green');

  fb_intercept2('HH\Lib\C\contains', 'bar');
}
