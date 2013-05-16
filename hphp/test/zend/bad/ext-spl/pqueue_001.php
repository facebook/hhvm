<?php
$pq = new SplPriorityQueue();

// errors
try {
    $pq->extract();
} catch (RuntimeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}

$pq->insert("a", 1);
$pq->insert("b", 2);
$pq->insert("c", 0);

foreach ($pq as $k=>$v) {
    echo "$k=>".print_r($v, 1)."\n";
}

echo "EXTR_BOTH\n";

$pq1 = new SplPriorityQueue();
$pq1->setExtractFlags(SplPriorityQueue::EXTR_BOTH);

$pq1->insert("a", 1);
$pq1->insert("b", 2);
$pq1->insert("c", 0);

foreach ($pq1 as $k=>$v) {
    echo "$k=>".print_r($v, 1)."\n";
}

echo "EXTR_DATA\n";

$pq2 = new SplPriorityQueue();
$pq2->setExtractFlags(SplPriorityQueue::EXTR_DATA);

$pq2->insert("a", 1);
$pq2->insert("b", 2);
$pq2->insert("c", 0);

foreach ($pq2 as $k=>$v) {
    echo "$k=>".print_r($v, 1)."\n";
}

echo "EXTR_PRIORITY\n";

$pq3 = new SplPriorityQueue();
$pq3->setExtractFlags(SplPriorityQueue::EXTR_PRIORITY);

$pq3->insert("a", 1);
$pq3->insert("b", 2);
$pq3->insert("c", 0);

foreach ($pq3 as $k=>$v) {
    echo "$k=>".print_r($v, 1)."\n";
}

?>
===DONE===
<?php exit(0); ?>