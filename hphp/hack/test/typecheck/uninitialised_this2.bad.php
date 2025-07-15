<?hh

class MyException extends Exception {
  public function __construct(mixed $_) {
    parent::__construct();
  }
}

class C {
  public int $i;
  public function __construct() {
    throw new MyException($this);
  }
}
