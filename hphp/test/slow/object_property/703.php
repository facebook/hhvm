<?hh

function test($x, $v) :mixed{
  var_dump($x->$v = 1);
}

<<__EntryPoint>>
function main_703() :mixed{
  test(true, "");
  test(true, "\0foo");
  test(new stdClass(), "");
}
