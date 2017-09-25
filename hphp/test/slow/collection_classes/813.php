<?hh

$v = new Vector;
$v[] = 'c';
$v[] = 'a';
$v[] = 'b';
sort($v);
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
ksort($m);
foreach ($m as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
echo "------------------------\n";
asort($m);
foreach ($m as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
