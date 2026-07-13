<?hh

function test($x, $v) :mixed{
  $r = is_object($x) ? 1 : null; $x->$v = 1; var_dump($r);
}

<<__EntryPoint>>
function main_703() :mixed{
  test(true, "");
  test(true, "\0foo");
  test(new stdClass(), "");
}
