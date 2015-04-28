<?hh // strict

class MyException extends Exception {
  public function __construct() {
    parent::__construct('dummy');
  }
}

function f(): void {
  throw new MyException();
}

function g(): void {
  throw new Exception('msg');
}
