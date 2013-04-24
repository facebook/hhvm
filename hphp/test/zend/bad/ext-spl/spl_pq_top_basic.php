<?php

$priorityQueue = new SplPriorityQueue();

$priorityQueue->insert("a", 1);
$priorityQueue->insert("b", 2);
$priorityQueue->insert("c", 0);

echo "EXTR DEFAULT",PHP_EOL;
echo "value: ",$priorityQueue->top(),PHP_EOL;

$priorityQueue->setExtractFlags(SplPriorityQueue::EXTR_PRIORITY);
echo "EXTR_PRIORITY",PHP_EOL;
echo "priority: ",$priorityQueue->top(),PHP_EOL;

$priorityQueue->setExtractFlags(SplPriorityQueue::EXTR_BOTH);
echo "EXTR_BOTH",PHP_EOL;
print_r($priorityQueue->top());

echo "EXTR_DATA",PHP_EOL;
$priorityQueue->setExtractFlags(SplPriorityQueue::EXTR_DATA);
echo "value: ",$priorityQueue->top(),PHP_EOL;
?>