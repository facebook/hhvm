<?php

function main($bits) {

  for ($tries = 0; $tries < 5; ++$tries) {
    $baseMemory = memory_get_usage();

    for ($i = 0; $i < 500000; ++$i) {
      $bits = $bits ^ $bits;
      $bits ^= $bits;
    }

    if (memory_get_usage() == $baseMemory) {
      echo "Usage is flat\n";
      return;
    }
  }

  echo "Usage didn't flatten out after 5 tries\n";
}

main('b613679a0814d9ec772f95d778c35fc5ff1697c493715653c6c712144292c5ad');
