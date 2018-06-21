<?hh //strict

function test(bool $b): array<string> {
  $a = array();
  if ($b) {
    $a[4] = 'aaa';
  }
  return $a;
}
