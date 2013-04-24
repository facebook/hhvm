<?php
  $h = new SplMaxHeap();
  //Line below should throw a warning as no args are expected
  $h->recoverFromCorruption("no args");
?>