<?php

function main(array $x = null) {
  if ($x) {
    echo is_array($x);
    echo "\n";
  } else {
    echo "empty or not array\n";
  }
}

main(array());
main(array(1,2,3));
