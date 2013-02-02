<?php

function main($x, $y) {
  while ($x < $y) {
    echo $x++ . "\n";
  }
}

main(1, 4);

echo "Done\n";
