<?php
  $d="worse ";
  $e="great";
  fb_taint($e);
  $f=$d . $e;
  echo($f); // tainted
  fb_taint($e);
  echo($e); // tainted
  echo($f); // tainted
