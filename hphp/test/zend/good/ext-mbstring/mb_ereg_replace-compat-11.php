<?php
/* (counterpart: ext/standard/tests/reg/014.phpt) */
  $a="a\\2bxc";
  echo mb_ereg_replace("a(.*)b(.*)c","\\1",$a);
?>