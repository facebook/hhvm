<?php
class FooException extends Exception {
  protected $code = 123;
  public function getCode() {
    return $this->code;
  }
}
$e = new FooException();
var_dump($e->getCode());