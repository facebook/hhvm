<?hh

function foo(): void {
  for (; true; $x = 1) {
    $y = $x;
  }
  echo $x;
}
