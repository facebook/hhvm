<?php
$heap = new SplMinHeap();
$heap->insert([1, 1]);
$heap->insert([2, 2]);
$heap->insert([1, 2]);
$heap->insert([2, 1]);
foreach ($heap as $item) {
  echo implode(' ', $item) . "\n";
}

$heap = new SplMaxHeap();
$heap->insert([1, 1]);
$heap->insert([2, 2]);
$heap->insert([1, 2]);
$heap->insert([2, 1]);
foreach ($heap as $item) {
  echo implode(' ', $item) . "\n";
}
