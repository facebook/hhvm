<?hh

function main() :mixed{
  $iToday = new DateTimeImmutable('today');
  $mToday = new DateTime('today');
  var_dump($iToday == $mToday);
}

<<__EntryPoint>>
function main_date_time_interface_different_class_equals() :mixed{
date_default_timezone_set('UTC');

main();
}
