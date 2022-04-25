<?hh

function f(string $dynamic_key): void {
  $d = dict['a' => 42];
  $d['b'] = true;

  $d[$dynamic_key];

  inspect($d);
}
