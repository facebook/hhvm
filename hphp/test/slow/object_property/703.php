<?hh

function test($x, $v) {
  var_dump($x->$v = 1);
}

<<__EntryPoint>>
function main_703() {
  test(true, "");
  test(true, "\0foo");
  test(new stdClass(), "");
}
