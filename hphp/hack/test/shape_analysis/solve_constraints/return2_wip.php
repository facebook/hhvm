<?hh
// expect return hint: shape('a' => int, 'b' => int)
function foo(): dict<string, mixed> {
  $d = bar();
  $d['b'] = 1;
  return $d;
}

function bar(): dict<string, mixed> {
  return dict['a' => 1];
}
