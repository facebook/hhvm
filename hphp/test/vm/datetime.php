<?php

function main() {
  try {
    new DateTimeZone('something');
  } catch (Exception $e) {
    echo "bad datetime\n";
  }
}
main();
