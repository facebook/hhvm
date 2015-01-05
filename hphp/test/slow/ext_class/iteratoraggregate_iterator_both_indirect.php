<?php
class IA implements IteratorAggregate {
  public function getIterator(){}
}

class Fatal extends IA implements Iterator {}
