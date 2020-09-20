<?hh

function main($s) {
  var_dump(wordwrap($s, 10));
}



<<__EntryPoint>>
function main_builtin_defaults() {
main("hello goodbye");
}
