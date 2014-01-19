<?php
/* (counterpart: ext/standard/tests/reg/008.phpt) */
  echo mb_ereg_replace("([a-z]*)([-=+|]*)([0-9]+)","\\3 \\1 \\2\n","abc+-|=123");
?>