<?hh

function f() {
  $mp1 = Map { 'a' => 1, 2 => 'b', 'c' => array()};
  $mp2 = Map {};
  $mp3 = Map {} ;
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
