<?php


<<__EntryPoint>>
function main_directory_iterator_ctor_emtpystring() {
try {
  new DirectoryIterator("");
} catch (RuntimeException $e) {
  echo "RuntimeException: " . $e->getMessage() . "\n";
}
}
