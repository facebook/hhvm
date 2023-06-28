<?hh

function test() :mixed{
  $m = Map { '42' => '65', 42 => 65 };
  var_dump($m, $m->toDArray());
  $s = Set { '42', 42 };
  var_dump($s, $s->toDArray());
}


<<__EntryPoint>>
function main_dup_keys() :mixed{
test();
}
