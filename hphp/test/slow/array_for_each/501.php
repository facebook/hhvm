<?php

function main() {
  echo "Entering main\n";
  foreach (array(1 => 1) as $k => $v) {
    break;
  }
  echo "Leaving main\n";
}
main();
