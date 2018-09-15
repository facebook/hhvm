<?hh

function &returns_ref(array<string, ?int> $arr): ?int {
  return $arr['k'];
}

function takes_ref(?int &$v): void {
  $v = 100;
}

function test(): void {
  $x = array('k' => null);
  takes_ref(&returns_ref($x));
  var_dump($x);
}
