<?php
  $d="worse ";
  $e="great";
  fb_taint($e);
  $f=$d . $e;
  $f .= " really great";
  echo($f); // tainted
