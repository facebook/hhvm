<?php

function main($arr) {
  if ($arr[1 << 33]) {
    var_dump($arr[1 << 33]);
  } else {
    echo "no\n";
  }
}

$packed = array(1,2,3);
$mixed = array((1 << 33) => array("value"));

main($packed);
main($mixed);
main($mixed);
main($mixed);
