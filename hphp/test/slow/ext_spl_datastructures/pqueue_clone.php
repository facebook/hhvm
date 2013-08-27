<?php

$queue = new SplPriorityQueue();
$queue->insert('first', 1);
$queue->insert('second', 2);
$queue->insert('third', 3);

$clone = clone $queue;

echo "Queue items: " . $queue->count() . PHP_EOL;
echo "Clone items: " . $clone->count() . PHP_EOL;

echo "Queue:".PHP_EOL;
for ($i = 0; $i < 3; $i++) {
  echo ' ' . $queue->extract() . PHP_EOL;
}

echo "Queue items: " . $queue->count() . PHP_EOL;
echo "Clone items: " . $clone->count() . PHP_EOL;

echo "Clone:".PHP_EOL;
for ($i = 0; $i < 3; $i++) {
  echo ' ' . $clone->extract() . PHP_EOL;
}
