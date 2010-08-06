<?php
  $d="worse ";
  $e="great";
  fb_set_taint($e, 0x1);
  $f=$d . $e;
  $f .= " really great";
  echo($f); // tainted
  $foo = " foo";
  $f .= $foo;
  echo($f); // tainted
