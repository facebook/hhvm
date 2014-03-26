<?php

class MyException extends Exception {}

try {
  ezc_throw('MyException');
} catch (Exception $e) {
  print "Caught " . get_class($e) . "\n";
}
try {
  ezc_throw('Exception');
} catch (Exception $e) {
  print "Caught " . get_class($e) . "\n";
}

