<?php
/* (counterpart: ext/standard/tests/reg/004.phpt) */
  $a="This is a nice and simple string";
  if (mb_ereg(".*nice and simple.*",$a)) {
    echo "ok\n";
  }
  if (!mb_ereg(".*doesn't exist.*",$a)) {
    echo "ok\n";
  }
?>