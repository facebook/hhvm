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
foreach ($vec->lazy()->keys()->map($mapFn)->filter($filtFn) as $x) {
  var_dump($x);
}
echo "------------------------\n";
$mp = Map {
0 => 'a', 3 => 'b', 6 => 'c'}
;
foreach ($mp->lazy()->keys()->map($mapFn)->filter($filtFn) as $x) {
  var_dump($x);
}
echo "------------------------\n";
$smp = StableMap {
0 => 'a', 3 => 'b', 6 => 'c', 9 => 'd'}
;
foreach ($smp->lazy()->keys()->map($mapFn)->filter($filtFn) as $x) {
  var_dump($x);
}
echo "------------------------\n";
$v = Vector {
0, 1, 2, 3, 4}
;
$iterable = $v->lazy()
              ->map(function ($x) {
 return $x+1;
 }
)
              ->filter(function ($x) {
 return $x % 2 == 0;
 }
);
foreach ($iterable as $v1) {
  foreach ($iterable as $v2) {
    echo "$v1 $v2\n";
  }
}
