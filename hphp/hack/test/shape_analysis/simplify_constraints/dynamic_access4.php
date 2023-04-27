<?hh

function f(bool $b, string $dynamic_key): void {
  $d = dict['a' => 42];
  $d['b'] = true;

  if ($b) {
    $d[$dynamic_key];
  }

  $d['c'] = 3.14;

  inspect($d);
}
