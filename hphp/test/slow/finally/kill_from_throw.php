<?hh

function foo() {
  try {
    try {
      return 0;
    } finally {
      throw new Exception('lol');
    }
  } catch (Exception $e) {
    var_dump("catch");
  }
  var_dump("after");
}

<<__EntryPoint>>
function main_kill_from_throw() {
var_dump(foo());
}
