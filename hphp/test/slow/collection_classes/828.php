<?hh

function f() {
  $obj = new stdClass();
  $a = Vector {
$obj, $obj}
;
  $a = unserialize(serialize($a));
  $a[0]->prop = 11;
  var_dump($a[1]->prop);
  $obj = new stdClass();
  $a = Map {
'a' => $obj, 'b' => $obj}
;
  $a = unserialize(serialize($a));
  $a['a']->prop = 22;
  var_dump($a['b']->prop);
  $obj = new stdClass();
  $a = StableMap {
'a' => $obj, 'b' => $obj}
;
  $a = unserialize(serialize($a));
  $a['a']->prop = 33;
  var_dump($a['b']->prop);
  $obj = new stdClass();
  $a = Pair {
$obj, $obj}
;
  $a = unserialize(serialize($a));
  $a[0]->prop = 44;
  var_dump($a[1]->prop);
}
f();
