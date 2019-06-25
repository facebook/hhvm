<?hh

class foo implements Throwable {
  function __construct($msg) {
    $this->msg = $msg;
  }
  function getMessage() { return $msg; }
  function getCode() {}
  function getFile() {}
  function getLine() {}
  function getTrace() {}
  function getTraceAsString() {}
  function getPrevious() {}
  function __toString() {}
}
<<__EntryPoint>> function main(): void {
try {
  try {
    throw new foo('try');
  } finally {
    throw new foo('finally');
  }
} catch (Throwable $t) {
  var_dump($t->getMessage());
}
}
