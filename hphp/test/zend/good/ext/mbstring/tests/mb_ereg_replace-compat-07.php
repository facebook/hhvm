<?php
/* (counterpart: ext/standard/tests/reg/010.phpt) */
  $a="abc122222222223";
  echo mb_ereg_replace("1(2*)3","\\1def\\1",$a);
?>