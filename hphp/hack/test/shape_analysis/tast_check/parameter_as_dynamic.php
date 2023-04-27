<?hh

function f(dict<string, mixed> $d): void {
  $d['k'] = 42;
  dict['b' => true];
}
