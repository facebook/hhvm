<?php

function main() {
  try {
    new DateTimeZone('something');
  } catch (Exception $e) {
    echo "bad datetime\n";
  }
  var_dump(DateTime::createFromFormat('2013-01-05', 'aoeu'));
}
main();

function specialIntervals() {
  $d = new DateTime('2015-05-05');
  echo 'Last day of the month of 2015-05-05 is ';
  echo $d->modify('last day of this month')->format('Y-m-d'), "\n";

  $d = new DateTime('2015-05-05');
  echo 'First day of the month of 2015-05-05 is ';
  echo $d->modify('first day of this month')->format('Y-m-d'), "\n";
}
specialIntervals();
