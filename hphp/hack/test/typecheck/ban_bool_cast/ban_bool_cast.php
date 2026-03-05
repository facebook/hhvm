<?hh

function test(mixed $x): void {
  $_ = (bool) $x;
  $_ = (int) $x;
  $_ = (string) $x;
}
