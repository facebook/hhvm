<?php


<<__EntryPoint>>
function main_rtit_cachingit_ctor() {
try {
  new RecursiveTreeIterator(new ArrayIterator([]));
} catch (Exception $e) {
  echo $e->getMessage() . "\n";
  // RecursiveCachingIterator expects param1 to be recit
}
}
