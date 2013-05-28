<?php

class X {
  public function foo($offset) {
    if (isset($this->__array[$offset])) {
      return $this->initializeOffset($offset);
    }
 else {
      return null;
    }
    return $this->__array[$offset];
  }
}
