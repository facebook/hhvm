<?hh

function test(bool $b): varray<string> {
  $a = dict[];
  if ($b) {
    $a[4] = 'aaa';
  }
  return $a;
}
