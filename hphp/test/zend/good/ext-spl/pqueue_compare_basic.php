<?php
$h = new SplPriorityQueue();
var_dump($h->compare(4, 5) < 0);
var_dump($h->compare(5, 5) == 0);
var_dump($h->compare(5, 4) > 0);
?>
===DONE===
<?php exit(0); ?>