<?php
$heap = new SplMaxHeap();
$heap->insert(42);
foreach ($heap as $key => $value) {
    var_dump($key);
    var_dump($value);
    break;
}

$heap = new SplMaxHeap();
$heap->insert(42);
var_dump($heap->key());
var_dump($heap->current());
?>