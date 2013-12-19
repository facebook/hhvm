<?hh

$v = Vector {
11, 42, 73}
;
foreach ($v->keys() as $x) {
  var_dump($x);
}
$mp = StableMap {
'a' => 1, 2 => 'b', 'z' => 9}
;
foreach ($mp->keys() as $x) {
  var_dump($x);
}
var_dump(new Vector($mp->keys()));
