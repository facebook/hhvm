<?php

class RecursiveCallbackFilterIterator extends CallbackFilterIterator
  implements OuterIterator, RecursiveIterator {


  public function __construct(\RecursiveIterator $it, callable $callback) {
    parent::__construct($it, $callback);
  }

  public function getChildren() {
    return new RecursiveCallbackFilterIterator(
      $this->getInnerIterator()->getChildren(),
      $this->callback
    );
  }

  public function hasChildren() {
    return $this->getInnerIterator()->hasChildren();
  }

}
