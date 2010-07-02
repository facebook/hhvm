<?php
  $d="worse ";
  $e="great";
  fb_taint($e);
  $f=$d . $e;
  $f .= " really great";
  $foo = " foo";
  $f .= $foo;
  fb_untaint($f);
  $f .= $foo;
  echo($f); // not tainted
