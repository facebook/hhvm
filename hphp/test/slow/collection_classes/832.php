<?hh

$mapFn = function ($x) {
 return $x*3+1;
 }
;
$vec = Vector {
2, 4, 6, 8}
;
foreach ($vec->items()->map($mapFn) as $x) {
  var_dump($x);
}
echo "------------------------\n";
$mapFn = function ($t) {
 return Pair {
$t[0]*3+1, $t[1]}
;
 }
;
$mp = Map {
2 => 'a'}
;
foreach ($mp->items()->map($mapFn) as $t) {
  var_dump($t[0], $t[1]);
}
echo "------------------------\n";
$smp = StableMap {
2 => 'a', 4 => 'b', 6 => 'c', 8 => 'd'}
;
foreach ($smp->items()->map($mapFn) as $t) {
  var_dump($t[0], $t[1]);
}
