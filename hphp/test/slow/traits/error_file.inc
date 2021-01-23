<?hh

trait T {
  use U;
  function g() {
    try {
      echo $x;
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
  }
}
