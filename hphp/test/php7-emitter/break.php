<?php

for ($i = 0; $i < 3; ++$i) {
  $x = 0;
  while (true) {
    if (++$x > 10)
      break;
  }
  echo $x . "\n";
}
