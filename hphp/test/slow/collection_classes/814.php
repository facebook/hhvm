<?hh

function cmp($x, $y) {
  if ($x < $y) return 1;
  if ($x > $y) return -1;
  return 0;
}
$v = new Vector;
$v[] = 'c';
$v[] = 'a';
$v[] = 'b';
usort($v, 'cmp');
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
uksort($m, 'cmp');
foreach ($m as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
echo "------------------------\n";
uasort($m, 'cmp');
foreach ($m as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
