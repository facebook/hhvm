<?php
  $d="worse ";
  $e="great";
  fb_set_taint($e, 0x1);
  $f=$d . $e;
  $f .= " really great";
  $foo = " foo";
  $f .= $foo;
  fb_unset_taint($f, 0x1);
  $f .= $foo;
  echo($f); // not tainted
