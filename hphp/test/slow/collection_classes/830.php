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
foreach ($vec->lazy()->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$mp = Map {
'a' => 0, 'b' => 3, 'c' => 6}
;
foreach ($mp->lazy()->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$smp = StableMap {
'a' => 0, 'b' => 3, 'c' => 6, 'd' => 9}
;
foreach ($smp->lazy()->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$pair = Pair {
0, 3}
;
foreach ($pair->lazy()->map($mapFn)->filter($filtFn) as $k => $v) {
  var_dump($k, $v);
}
