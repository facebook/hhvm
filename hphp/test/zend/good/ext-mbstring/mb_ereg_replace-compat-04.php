<?php
/* (counterpart: ext/standard/tests/reg/006.phpt) */
  $a="This is a nice and simple string";
  echo mb_ereg_replace("^This","That",$a);
?>