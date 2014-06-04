<?php

class ParentIterator extends RecursiveFilterIterator
 implements RecursiveIterator, OuterIterator {

  public function accept() {
    return $this->getInnerIterator()->hasChildren();
  }
}
