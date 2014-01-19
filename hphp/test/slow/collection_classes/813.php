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
$s = new StableMap;
$s['w'] = 2;
$s['v'] = 4;
$s['y'] = 3;
$s['x'] = 5;
$s['z'] = 1;
ksort($s);
foreach ($s as $key => $val) {
  echo $key . ' ' . $val . "\n";
}
echo "------------------------\n";
asort($s);
foreach ($s as $key => $val) {
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
