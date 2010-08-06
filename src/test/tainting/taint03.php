<?php
  $a="bad";
  fb_set_taint($a, 0x1);
  $c=$a . "good";
  echo $c; // tainted
