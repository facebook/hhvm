<?hh

function nextBit(inout int $x): bool {
  $ret = (bool)($x & 1);
  $x >>= 1;
  return $ret;
}

function testOne(int $case): void {
  $left = nextBit(inout $case) ? Map{} : dict[];
  $right = nextBit(inout $case) ? Map{} : dict[];
  if (nextBit(inout $case)) $left['a']  = 'a';
  if (nextBit(inout $case)) $left[1]    = 'i';
  if (nextBit(inout $case)) $left['1']  = 's';
  if (nextBit(inout $case)) $right['a'] = 'x';
  if (nextBit(inout $case)) $right[1]   = 'y';
  if (nextBit(inout $case)) $right['1'] = 'z';

  // skip boring cases
  if (count($left) === 0 || count($right) === 0) return;

  $res = array_intersect_key($left, $right);
  echo
    $left is Map<_>    ? 'm' : 'd',
    isset($left['a'])  ? 'a' : ' ',
    isset($left[1])    ? 'i' : ' ',
    isset($left['1'])  ? 's' : ' ',
    ' + ',
    $right is Map<_>   ? 'm' : 'd',
    isset($right['a']) ? 'a' : ' ',
    isset($right[1])   ? 'i' : ' ',
    isset($right['1']) ? 's' : ' ',
    ' = \'',
    $res['a'] ?? ' ',
    $res[1]   ?? ' ',
    $res['1'] ?? ' ',
    "'\n";
}

<<__EntryPoint>>
function main(): void {
  for ($i = 0; $i < 256; $i++) testOne($i);
}
