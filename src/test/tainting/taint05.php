<?php
  $d="worse ";
  $e="great";
  fb_set_taint($e, 0x1);
  $f=$d . $e;
  echo($f); // tainted
  fb_set_taint($e, 0x1);
  echo($e); // tainted
  echo($f); // tainted
