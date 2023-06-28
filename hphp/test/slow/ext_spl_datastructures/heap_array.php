<?hh

<<__EntryPoint>>
function main_heap_array() :mixed{
$heap = new SplMinHeap();
$heap->insert(varray[1, 1]);
$heap->insert(varray[2, 2]);
$heap->insert(varray[1, 2]);
$heap->insert(varray[2, 1]);
foreach ($heap as $item) {
  echo implode(' ', $item) . "\n";
}

$heap = new SplMaxHeap();
$heap->insert(varray[1, 1]);
$heap->insert(varray[2, 2]);
$heap->insert(varray[1, 2]);
$heap->insert(varray[2, 1]);
foreach ($heap as $item) {
  echo implode(' ', $item) . "\n";
}
}
