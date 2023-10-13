<?hh

function test(bool $b): varray<string> {
  $a = darray[];
  if ($b) {
    $a[4] = 'aaa';
  }
  return $a;
}
