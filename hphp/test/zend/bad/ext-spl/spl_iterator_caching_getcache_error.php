<?php
$i = new ArrayIterator(array(1,1,1,1,1));
$i = new CachingIterator($i);
try {
  $i->getCache();
  echo "Should have caused an exception";
} catch (BadMethodCallException $e) {
  echo "Exception raised\n";
}

?>
===DONE===