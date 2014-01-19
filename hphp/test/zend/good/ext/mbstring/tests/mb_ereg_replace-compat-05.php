<?php
/* (counterpart: ext/standard/tests/reg/007.phpt) */
  $a="abcd";
  $b=mb_ereg_replace("abcd","",$a);
  echo "strlen(\$b)=".strlen($b);
?>