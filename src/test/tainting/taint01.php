<?php
  $a = "good\n";
  if(fb_get_taint($a) & 0x1){
    echo "a is tainted\n";
  } else {
    echo "a is not tainted\n";
  }
  echo $a; // not tainted
