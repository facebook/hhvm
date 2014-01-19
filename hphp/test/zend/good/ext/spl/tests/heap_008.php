<?php
$h = new SplMaxHeap();

$h->insert(1);
$h->insert(5);
$h->insert(0);
$h->insert(4);

var_dump($h);
?>
===DONE===
<?php exit(0); ?>