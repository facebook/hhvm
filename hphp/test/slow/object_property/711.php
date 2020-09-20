<?hh

function test($x, $v) {
  var_dump($x->$v += 1);
}

<<__EntryPoint>>
function main_711() {
  test(true, "");
  test(true, "\0foo");
  test(new stdClass(), "");
}
