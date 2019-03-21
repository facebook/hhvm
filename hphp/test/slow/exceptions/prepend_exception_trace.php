<?hh

class PrependingException extends Exception {
  public function prepend(varray<darray<string,mixed>> $something) {
    $this->__prependTrace($something);
  }
}


<<__EntryPoint>>
function main_prepend_exception_trace() {
  $exception = new PrependingException();
  $exception->prepend(varray[darray['prepended_key' => 'prepended_value']]);
  var_dump($exception->getTrace()[0]);
  var_dump($exception->getTraceAsString());
}
