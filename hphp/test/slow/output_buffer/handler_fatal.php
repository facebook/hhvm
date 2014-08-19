<?php

function main() {
  ob_start(function($str) {
    bar();
    return $str.'!!!';
  });

  echo 'garbanzo beans';

  ob_end_flush();

  echo "DON'T PRINT ME!";
}

main();
