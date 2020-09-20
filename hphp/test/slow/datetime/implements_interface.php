<?hh

function main() {
  var_dump((new DateTime()) is DateTimeInterface);
}

<<__EntryPoint>>
function main_implements_interface() {
date_default_timezone_set('UTC');

main();
}
