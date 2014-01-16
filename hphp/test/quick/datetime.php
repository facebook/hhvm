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
