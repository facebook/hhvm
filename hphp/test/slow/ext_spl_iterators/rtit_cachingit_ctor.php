<?php

try {
  new RecursiveTreeIterator(new ArrayIterator([]));
} catch (Exception $e) {
  echo $e->getMessage() . "\n";
  // RecursiveCachingIterator expects param1 to be recit
}
