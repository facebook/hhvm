<?hh

function main() {
  try {
    var_dump("try");
  } finally {
    var_dump("finally");
  }
}

<<__EntryPoint>>
function main_fall_through() {
main();
}
