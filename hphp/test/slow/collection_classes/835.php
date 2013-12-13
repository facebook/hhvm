<?hh

$c1 = StableMap {
'a' => 0, 'b' => 3, 'c' => 6, 'd' => 9}
;
$c2 = Vector {
1, 4, 7}
;
foreach ($c1->zip($c2) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$c1 = Vector {
1, 4, 7, 10}
;
$c2 = StableMap {
'a' => 0, 'b' => 3, 'c' => 6}
;
foreach ($c1->zip($c2) as $k => $v) {
  var_dump($k, $v);
}
echo "------------------------\n";
$c1 = Pair {
1, 4}
;
$c2 = StableMap {
'a' => 0, 'b' => 3, 'c' => 6}
;
foreach ($c1->zip($c2) as $k => $v) {
  var_dump($k, $v);
}
