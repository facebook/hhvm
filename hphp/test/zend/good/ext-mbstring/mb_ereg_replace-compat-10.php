<?php
/* (counterpart: ext/standard/tests/reg/013.phpt) */
  $a="abc123";
  echo mb_ereg_replace("123","def\\g\\\\hi\\",$a);
?>