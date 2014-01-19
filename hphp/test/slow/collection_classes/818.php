<?hh

function f() {
  $x = StableMap {
'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4}
;
  unset($x['a']);
  unset($x['c']);
  foreach ($x as $k => $v) {
    echo $k . ' ' . $v . "\n";
  }
}
f();
