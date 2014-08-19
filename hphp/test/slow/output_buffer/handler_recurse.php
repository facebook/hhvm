<?php

function ob_handler($str) {
  return $str.'???';
}

function main() {
  $handler = function($str) {
    ob_start('ob_handler');
    echo $str.'!!!';
    $ret = ob_get_contents();
    ob_end_flush();
    return $ret;
  };

  ob_start($handler);
  echo 'garbanzo beans';
  ob_end_flush();

  echo "DON'T PRINT ME";
}

main();
