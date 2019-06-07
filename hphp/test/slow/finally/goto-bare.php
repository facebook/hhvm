<?hh

function foo() {
  try {
    goto label;
  } finally {
    var_dump("finally1");
  }

  return "wrong!";

label:
  var_dump("label");
  return "return2";
}


<<__EntryPoint>>
function main_goto_bare() {
var_dump(foo());
}
