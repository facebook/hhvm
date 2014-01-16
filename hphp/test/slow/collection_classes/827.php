<?hh

function f() {
  $mp1 = StableMap {
'a' => 1, 2 => 'b', 'c' => array()}
;
  $mp2 = StableMap {
}
;
  $mp3 = StableMap {
}
;
  foreach ($mp1->items() as $t) {
    $mp2->add($t);
  }
  var_dump($mp2);
  foreach ($mp1->items() as $t) {
    $mp3[] = $t;
  }
  var_dump($mp3);
}
f();
