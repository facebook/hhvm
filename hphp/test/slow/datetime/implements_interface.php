<?hh

function main() :mixed{
  var_dump((new DateTime()) is DateTimeInterface);
}

<<__EntryPoint>>
function main_implements_interface() :mixed{
date_default_timezone_set('UTC');

main();
}
