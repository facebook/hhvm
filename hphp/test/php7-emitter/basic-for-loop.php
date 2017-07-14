<?php

for ($x = 0; $x < 10; ++$x) {
  for ($y = 0; $y < $x; ++$y) {
    for ($z = 0; $z < $y; ++$z) {
      echo '*';
    }
    echo "\n";
  }
}
