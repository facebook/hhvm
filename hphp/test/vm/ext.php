<?php

function main() {
  $date = new DateTime("now");
  $date->setTimezone(new DateTimeZone('America/Los_Angeles'));
  $tz = $date->getTimezone();
  print $tz->getName() . "\n";
}
main();

