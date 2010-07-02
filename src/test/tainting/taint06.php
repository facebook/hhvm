<?php
  $d="worse ";
  $e="great";
  fb_taint($e);
  $f=$d . $e;
  fb_untaint($f);
  echo($e); // tainted
  echo($f); // not tainted
