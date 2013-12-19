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
$s = new StableMap;
$s['w'] = 2;
$s['v'] = 4;
$s['y'] = 3;
$s['x'] = 5;
$s['z'] = 1;
uksort($s, 'cmp');
foreach ($s as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
echo "------------------------\n";
uasort($s, 'cmp');
foreach ($s as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
