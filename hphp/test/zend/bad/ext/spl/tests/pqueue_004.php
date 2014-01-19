<?php
$pq = new SplPriorityQueue();

$pq->insert("a", 0);
$pq->insert("b", 1);
$pq->insert("c", 5);
$pq->insert("d", -2);

var_dump($pq);
?>
===DONE===
<?php exit(0); ?>