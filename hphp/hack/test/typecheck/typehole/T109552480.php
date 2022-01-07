<?hh

function wut(int $v) : int {
  $s = shape('k' => 42);
  switch ($v) {
    case 10:
      Shapes::removeKey(inout $s, 'k');
      $s['k'] = 'hello';
      // FALLTHROUGH
    case 20:
      // FALLTHROUGH
    case 30:
      return $s['k'];
  }

  return 42;
}

<<__EntryPoint>>
function main(): void {
  wut(10);
}
