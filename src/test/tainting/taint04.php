<?php
  $a="bad";
  fb_set_taint($a, 0x1);
  $c=$a . "good";
  echo $c; // tainted
  fb_unset_taint($c, 0x1);
  echo $c; // not tainted
