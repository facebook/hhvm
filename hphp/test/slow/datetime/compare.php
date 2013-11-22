<?php

function main() {
  date_default_timezone_set('UTC');
  $tz = new DateTimeZone(date_default_timezone_get());
  $now = new DateTime(null, $tz);
  $tests = array(
    new DateTime('-1day', $tz),
    new DateTime('+1day', $tz),
  );

  foreach ($tests as $test) {
    var_dump($now < $test);
    var_dump($now == $test);
    var_dump($now > $test);
  }
}
main();
