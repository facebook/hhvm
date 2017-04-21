<?php

class PrependingException extends Exception {
  public function prepend(array $something) {
    $this->__prependTrace($something);
  }
}

$exception = new PrependingException();
$exception->prepend(array('prepended_value'));
echo $exception->getTrace()[0];
