<?hh // partial

function test(bool $b): void {
  if ($b) {
    $x = 1;
  }
  if (isset($x)) {
    echo $x;
  }
}
