<?hh

function test($x, $v) :mixed{
  $x->$v += 1; $r = is_object($x) ? $x->$v : null; var_dump($r);
}

<<__EntryPoint>>
function main_711() :mixed{
  test(true, "");
  test(true, "\0foo");
  test(new stdClass(), "");
}
