<?hh

<<__EntryPoint>>
function main_heap_array() :mixed{
$heap = new SplMinHeap();
$heap->insert(vec[1, 1]);
$heap->insert(vec[2, 2]);
$heap->insert(vec[1, 2]);
$heap->insert(vec[2, 1]);
foreach ($heap as $item) {
  echo implode(' ', $item) . "\n";
}

$heap = new SplMaxHeap();
$heap->insert(vec[1, 1]);
$heap->insert(vec[2, 2]);
$heap->insert(vec[1, 2]);
$heap->insert(vec[2, 1]);
foreach ($heap as $item) {
  echo implode(' ', $item) . "\n";
}
}
