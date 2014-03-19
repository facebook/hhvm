<?php
date_default_timezone_set('UTC');

function main() {
  $iToday = new DateTimeImmutable('today');
  $mToday = new DateTime('today');
  var_dump($iToday == $mToday);
}

main();
