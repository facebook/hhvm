<?php

for ($i1 = 0;
 $i1 < 2;
 $i1++) {
  for ($i2 = 0;
 $i2 < 2;
 $i2++) {
    switch ($i2 % 2) {
      case 0:        continue;
        break;
    }
    print "[" . $i2 . "]\n";
  }
  print $i1 . "\n";
}
