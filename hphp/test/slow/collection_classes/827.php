<?hh

function f() :mixed{
  $mp1 = Map { 'a' => 1, 2 => 'b', 'c' => vec[]};
  $mp2 = Map {};
  $mp3 = Map {} ;
  foreach ($mp1->items() as $t) {
    $mp2->add($t);
  }
  var_dump($mp2);
  foreach ($mp1->items() as $t) {
    $mp3->add($t);
  }
  var_dump($mp3);
}

<<__EntryPoint>>
function main_827() :mixed{
f();
}
