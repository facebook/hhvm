<?hh

function cmp($x, $y) {
  if ($x < $y) return 1;
  if ($x > $y) return -1;
  return 0;
}

<<__EntryPoint>>
function main_814() {
$v = new Vector;
$v[] = 'c';
$v[] = 'a';
$v[] = 'b';
usort(inout $v, fun('cmp'));
foreach ($v as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
echo "------------------------\n";
$m = new Map;
$m['w'] = 2;
$m['v'] = 4;
$m['y'] = 3;
$m['x'] = 5;
$m['z'] = 1;
uksort(inout $m, fun('cmp'));
foreach ($m as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
echo "------------------------\n";
uasort(inout $m, fun('cmp'));
foreach ($m as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
}
