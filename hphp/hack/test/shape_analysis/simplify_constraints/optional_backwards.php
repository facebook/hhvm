<?hh

function f(dict<string, mixed> $d): void {
  $b = true;
  if ($b) {
    $d['a'];
    idx($d,'b');
    $d['c'];
  } else {
    idx($d,'a');
    idx($d,'b');
    $d['c'];
  }
}
