<?hh // strict

<<__Rx>>
function append(bool $x): void {
  if ($x) {
    $y = Vector {};
  } else {
    $y = Vector { 7 };
  }
  $y[] = 5;
}
