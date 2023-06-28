<?hh

function foo() :mixed{
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
function main_kill_from_throw() :mixed{
var_dump(foo());
}
