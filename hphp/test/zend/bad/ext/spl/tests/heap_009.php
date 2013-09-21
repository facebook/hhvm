<?php
function testForException( $heap )
{
    try
    {
        foreach( $heap as &$item );
    }
    catch( RuntimeException $e )
    {
        echo $e->getMessage(),"\n";
    }
}

// 1. SplMinHeap emtpy
$heap = new SplMinHeap;
testForException( $heap );

// 2. SplMinHeap non-emtpy
$heap = new SplMinHeap;
$heap->insert( 1 );
testForException( $heap );

// 3. SplMaxHeap emtpy
$heap = new SplMaxHeap;
testForException( $heap );

// 4. SplMaxHeap non-emtpy
$heap = new SplMaxHeap;
$heap->insert( 1 );
testForException( $heap );

// 5. SplPriorityQueue empty
$heap = new SplPriorityQueue;
testForException( $heap );

// 6. SplPriorityQueue non-empty
$heap = new SplPriorityQueue;
$heap->insert( 1, 2 );
testForException( $heap );

?>
==DONE==