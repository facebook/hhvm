<?hh

function test(bool $b, bool $c): void {
  if ($b) {
    if ($c) {
    }
    echo $x;
    $x = 1;
  } else {
    $x = 2;
  }
}
