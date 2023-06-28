<?hh

class foo implements Throwable {
  function __construct($msg) {
    $this->msg = $msg;
  }
  function getMessage() :mixed{ return $msg; }
  function getCode() :mixed{}
  function getFile() :mixed{}
  function getLine() :mixed{}
  function getTrace() :mixed{}
  function getTraceAsString() :mixed{}
  function getPrevious() :mixed{}
  function __toString()[] :mixed{}
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
