<?hh
<<__EntryPoint>> function main() {
$priorityQueue = new SplPriorityQueue();

try {
    $priorityQueue->top();
} catch (RuntimeException $e) {
    echo "Exception: ".$e->getMessage().PHP_EOL;
}
}
