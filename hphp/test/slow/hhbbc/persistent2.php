<?php

function main($z) {
  var_dump(function_exists($z, false));
  var_dump(function_exists($z, true));
  if (!function_exists('Z')) {
    echo "Loading...\n";
    require_once('persistent.inc');
  }

  var_dump(z());
}

main('Z');
