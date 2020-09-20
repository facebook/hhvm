<?hh

function main() {
  date_default_timezone_set('UTC');
  $tz = new DateTimeZone(date_default_timezone_get());
  $now = new DateTime('', $tz);
  $tests = varray[
    new DateTime('-1day', $tz),
    new DateTime('+1day', $tz),
  ];

  foreach ($tests as $test) {
    var_dump($now < $test);
    var_dump($now == $test);
    var_dump($now > $test);
  }

  // microseconds tests
  $base = DateTime::createFromFormat('U.u', '1448889063.3531');
  $tests = varray[
    DateTime::createFromFormat('U.u', '1448889063.3530'),
    DateTime::createFromFormat('U.u', '1448889063.3531'),
    DateTime::createFromFormat('U.u', '1448889063.3532')
  ];

  foreach ($tests as $test) {
    var_dump($base < $test);
    var_dump($base == $test);
    var_dump($base > $test);
  }
}

<<__EntryPoint>>
function main_compare() {
main();
}
