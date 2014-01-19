<?php
Class C {}

class D extends C implements Iterator {
  
  private $counter = 2;
  
  public function valid() {
    echo __METHOD__ . "($this->counter)\n";
    return $this->counter;    
  }
  
  public function next() {
    $this->counter--;   
    echo __METHOD__ . "($this->counter)\n";
  }
  
  public function rewind() {
    echo __METHOD__ . "($this->counter)\n";
  }
  
  public function current() {
    echo __METHOD__ . "($this->counter)\n";
  }
  
  public function key() {
    echo __METHOD__ . "($this->counter)\n";
  }
  
}

foreach (new D as $x) {}
?>