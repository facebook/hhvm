<?hh

$mapFn = function ($t) {
 return Pair {
$t[0]*3+1, $t[1]}
;
 }
;
$vec = Vector {
'a', 'b', 'c', 'd'}
;
foreach ($vec->kvzip()->map($mapFn) as $t) {
  var_dump($t[0], $t[1]);
}
echo "------------------------\n";
$mp = Map {
2 => 'a'}
;
foreach ($mp->kvzip()->map($mapFn) as $t) {
  var_dump($t[0], $t[1]);
}
echo "------------------------\n";
$smp = StableMap {
2 => 'a', 4 => 'b', 6 => 'c', 8 => 'd'}
;
foreach ($smp->kvzip()->map($mapFn) as $t) {
  var_dump($t[0], $t[1]);
}
echo "------------------------\n";
$pair = Pair {
'a', 'b'}
;
foreach ($pair->kvzip()->map($mapFn) as $t) {
  var_dump($t[0], $t[1]);
}
