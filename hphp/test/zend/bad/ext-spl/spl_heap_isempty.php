<?php
  $h = new SplMaxHeap();
  echo "Checking a new heap is empty: ";
  var_dump($h->isEmpty())."\n";
  $h->insert(2);
  echo "Checking after insert: ";
  var_dump($h->isEmpty())."\n";
  $h->extract();
  echo "Checking after extract: ";
  var_dump($h->isEmpty())."\n";
?>