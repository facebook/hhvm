<?hh

// expect return hint: `shape('a' => mixed)`
function foo(): dict<string, mixed> {
  $d = bar();
  $d['a']; // expect `Shapes::idx($d, 'a')`
  return $d;
}

function bar(): dict<string, mixed> {
  return dict[];
}
