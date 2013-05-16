<?php
class SplHeap2 extends SplHeap{

  public function compare() {
           return -parent::compare();
       }
}

$h = new SplHeap2;
$h->isEmpty(1);
?>