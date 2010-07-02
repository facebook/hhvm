<?php
  $a = "good\n";
  if(fb_is_tainted($a)){
    echo "a is tainted\n";
  } else {
    echo "a is not tainted\n";
  }
  echo $a; // not tainted
