<?hh

class MyException extends Exception {
  public function __construct() {
    parent::__construct('dummy');
  }
}

function f(): void {}

function g(): void {
  try {
  } catch (MyException $m) {
  } catch (Exception $e) {
  }
}
