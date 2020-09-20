<?hh


function main($s) {
  unset($s[0]);
}

<<__EntryPoint>>
function main_unset_string() {
main('yo');
}
