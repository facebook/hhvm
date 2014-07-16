<?hh

function test() {
  $m = Map { '42' => '65', 42 => 65 };
  var_dump($m, $m->toArray());
  $s = Set { '42', 42 };
  var_dump($s, $s->toArray());
}

test();
