<?hh // strict

function f(): void {
  $a = $b || $c = &$d || $e;
}
