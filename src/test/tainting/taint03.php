<?php
  $a="bad";
  fb_taint($a);
  $c=$a . "good";
  echo $c; // tainted
