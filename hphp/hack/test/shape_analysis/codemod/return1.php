<?hh

function f(): dict<string, mixed> {
  $d = dict['a' => 42];
  $d['b'] = true;
  return $d;
}
