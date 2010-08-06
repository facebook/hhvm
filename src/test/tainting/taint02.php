<?php
  $a="bad\n";
  fb_set_taint($a, 0x1);
  if(fb_get_taint($a) & 0x1){
    echo "a is tainted\n";
  } else {
    echo "a is not tainted\n";
  }
  echo $a; // tainted
