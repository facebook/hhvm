<?php

class MyHeap extends SplMaxHeap
{
    public function testCompare()
    {
        return parent::compare(1);
    }
}

$heap = new MyHeap();
$heap->testCompare();

?>