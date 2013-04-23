<?php
/* (counterpart: ext/standard/tests/reg/003.phpt) */
  $a="\\'test";
  echo mb_ereg_replace("\\\\'","'",$a);
?>