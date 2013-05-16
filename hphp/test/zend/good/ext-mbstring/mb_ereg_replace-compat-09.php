<?php
/* (counterpart: ext/standard/tests/reg/012.phpt) */
  $a="abc123";
  echo mb_ereg_replace("123",'def\1ghi',$a);
?>