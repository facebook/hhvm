<?php
  $a="bad\n";
  fb_taint($a);
  if(fb_is_tainted($a)){
    echo "a is tainted\n";
  } else {
    echo "a is not tainted\n";
  }
  echo $a; // tainted
