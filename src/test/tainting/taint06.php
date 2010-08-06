<?php
  $d="worse ";
  $e="great";
  fb_set_taint($e, 0x1);
  $f=$d . $e;
  fb_unset_taint($f, 0x1);
  echo($e); // tainted
  echo($f); // not tainted
