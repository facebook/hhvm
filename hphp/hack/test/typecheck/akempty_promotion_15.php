<?hh //strict

function test(bool $b): array<string> {
  $a = darray[];
  if ($b) {
    $a[4] = 'aaa';
  }
  return $a;
}
