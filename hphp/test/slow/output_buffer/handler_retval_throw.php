<?php

class Chickpea {
  public function __toString() {
    throw new Exception('chickpeas');
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
