<?php

class MyHeap extends SplHeap{

public function compare($a, $b){
return $a < $b;
}

}


$heap = new MyHeap();
var_dump($heap->isEmpty());
$heap->insert(1);
var_dump($heap->isEmpty());
$heap->extract();
var_dump($heap->isEmpty());
$heap->isEmpty('var');
?>