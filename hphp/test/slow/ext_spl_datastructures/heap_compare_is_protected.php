<?php
class SplMinHeap2 extends SplMinHeap {
  protected function compare($a, $b) {
    return -parent::compare($a,$b);
  }
}
$h = new SplMinHeap2();
$h->insert(1);
$h->insert(6);
var_dump($h->top());

class SplMaxHeap2 extends SplMaxHeap {
  protected function compare($a, $b) {
    return -parent::compare($a,$b);
  }
}
$h = new SplMaxHeap2();
$h->insert(1);
$h->insert(6);
var_dump($h->top());
