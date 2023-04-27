<?hh
// expect return hint: shape('a' => int)
function foo(): dict<string, mixed> {
  return bar();
}

function bar(): dict<string, mixed> {
  return dict['a' => 3];
}
