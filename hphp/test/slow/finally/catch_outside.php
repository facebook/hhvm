<?hh

function foo() {
  try {
    try {
      throw new Exception("foo");
    } finally {
      var_dump("lol");
    }
  } catch (Exception $e) {
    var_dump("oops");
  }
}

<<__EntryPoint>>
function main_catch_outside() {
foo();
}
