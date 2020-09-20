<?hh //partial

class Two extends One {
  public function foo() {
    return 2;
  }

  public function bar() {
    return 1;
  }

  public function usenum() {
    $x = new One();

    $x->noparameter(1);
  }
}

function throwException() {
  throw new \Exception("");
}

function throwExceptionIndirect() {
  throwException();
}
