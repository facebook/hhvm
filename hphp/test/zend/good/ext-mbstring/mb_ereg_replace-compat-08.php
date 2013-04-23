<?php
/* (counterpart: ext/standard/tests/reg/011.phpt) */
  $a="abc123";
  echo mb_ereg_replace("123","def\\0ghi",$a);
?>