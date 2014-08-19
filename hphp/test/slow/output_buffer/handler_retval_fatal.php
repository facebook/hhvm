<?php

class Chickpea {
  public function __toString() {
    chickpea();
    return 'chickpea';
  }
}

function main() {
  ob_start(function($str) {
    return new Chickpea();
  });

  echo 'garbanzo beans';

  ob_end_flush();

  echo "DON'T PRINT ME!";
}

main();
