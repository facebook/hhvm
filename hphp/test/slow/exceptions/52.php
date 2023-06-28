<?hh

class Exception1 extends Exception {
  public function __construct() {
    parent::__construct();
  }
}

class Exception2 extends Exception1 {
  public function exception2() :mixed{
    parent::__construct();
  }
}

function foo() :mixed{
  throw new Exception2();
}
<<__EntryPoint>>
function bar() :mixed{
  try {
    foo();
  }
  catch (Exception $exn) {
    $a = $exn->getTrace();
    foreach ($a as $k => $v) $a[$k]['file'] = 'string';
    var_dump($a);
    var_dump($exn->getLine());
  }
}
