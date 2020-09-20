<?hh

final class MyException extends Exception {
  public function __construct() {
    $this->__prependTrace(varray['garbage!']);
  }
}

function throws_my_exn() {
  throw new MyException(__FUNCTION__);
}

function throws_exn() {
  printf("");
  throw new Exception(__FUNCTION__);
}

<<__EntryPoint>>
function main() {
  try {
    throws_exn();
  } catch (Exception $e) {
    var_dump($e->getTrace());
  }
  try {
    throws_my_exn();
  } catch (MyException $e) {
    var_dump($e->getTrace());
  }
}
