<?php

try {
  new RecursiveCachingIterator(new ArrayIterator([]));
} catch (InvalidArgumentException $e) {
  echo $e->getMessage() . "\n";
}

new RecursiveCachingIterator(new ArrayObject([]));
