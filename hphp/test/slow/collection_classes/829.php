<?hh

$mapFn = function ($v) {
 return $v+1;
 }
;
$filtFn = function ($v) {
 return $v % 2 == 0;
 }
;
$vec = Vector {
0, 3, 6, 9}
;
foreach ($vec->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$mp = Map {
'a' => 0, 'b' => 3, 'c' => 6}
;
foreach ($mp->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$smp = StableMap {
'a' => 0, 'b' => 3, 'c' => 6, 'd' => 9}
;
foreach ($smp->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$pair = Pair {
0, 3}
;
foreach ($pair->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
