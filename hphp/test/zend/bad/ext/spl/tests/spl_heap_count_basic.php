<?php

class MyHeap extends SplHeap
{
    public function compare($a,$b)
    {
        return ($a < $b);
    }

    public function count() // override count to force failure
    {
        throw new Exception('Cause count to fail');
        return parent::count();
    }
}


$heap = new MyHeap();
$heap->insert(1);
count($heap);// refers to MyHeap->count() method

?>