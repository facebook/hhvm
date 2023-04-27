<?hh

function f(dict<string, mixed> $d, string $dynamic_key): void {
  $d['b'] = true;

  $d[$dynamic_key];

  inspect($d);
}
