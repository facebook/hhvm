<?hh

function f(): dict<string, mixed> {
  $d = dict['a' => 42];
  $e = dict['b' => true];
  return $d;
}
