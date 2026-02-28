<?hh

function f(): void {
  $a = $b || $c = &$d || $e;
}
