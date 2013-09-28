<?php
class MyHeap extends SplHeap
{
    public function compare($a, $b)
    {
        return $a < $b;
    }
}

$heap = new MyHeap();
$heap->insert(1,2);
?>