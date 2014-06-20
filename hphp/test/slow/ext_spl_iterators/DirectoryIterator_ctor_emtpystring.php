<?php

try {
  new DirectoryIterator("");
} catch (RuntimeException $e) {
  echo "RuntimeException: " . $e->getMessage() . "\n";
}
