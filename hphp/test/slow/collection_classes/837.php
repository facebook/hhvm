<?hh

$mapFn = function ($v) {
 return $v+1;
 }
;
$filtFn = function ($v) {
 return $v % 2 == 0;
 }
;
$st = Set {
0, 3}
;
var_dump($st->map($mapFn)->filter($filtFn));
foreach ($st->lazy()->map($mapFn)->filter($filtFn) as $v) {
  var_dump($v);
}
$st = new Set(Vector {
6, 9}
);
var_dump($st->map($mapFn)->filter($filtFn));
foreach ($st->items()->map($mapFn)->filter($filtFn) as $v) {
  var_dump($v);
}

