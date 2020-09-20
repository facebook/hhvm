<?hh <<__EntryPoint>> function main(): void {
$queue = new SplQueue();
// errors
try {
    $queue->dequeue();
} catch (RuntimeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}
try {
    $queue->shift();
} catch (RuntimeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}

// data consistency
$a = 2;
$queue->enqueue($a);
echo $queue->dequeue()."\n";

// peakable
$queue->enqueue(1);
$queue->enqueue(2);
echo $queue->top()."\n";

// iterable
foreach ($queue as $elem) {
    echo "[$elem]\n";
}

// countable
$queue->enqueue(NULL);
$queue->enqueue(NULL);
echo count($queue)."\n";
echo $queue->count()."\n";
var_dump($queue->dequeue());
var_dump($queue->dequeue());

// clonable
$queue->enqueue(2);
$queue_clone = clone $queue;
$queue_clone->dequeue();
echo count($queue)."\n";
echo "===DONE===\n";
}
