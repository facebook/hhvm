<?php
$i = new ArrayIterator(array(1,1,1,1,1));
$i = new CachingIterator($i,CachingIterator::FULL_CACHE);
foreach ($i as $value) {
  echo $i->count()."\n";
}
?>
===DONE===