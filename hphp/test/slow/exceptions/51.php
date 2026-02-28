<?hh

class MyException extends Exception {
  public function __construct() {
  }
}
function thrower() :mixed{
  throw new MyException();
}
<<__EntryPoint>> function main(): void {
try {
  thrower();
}
catch (Exception $exn) {
  $a = $exn->getTrace();
  foreach ($a as $k => $v) $a[$k]['file'] = 'string';
  var_dump($a);
  var_dump($exn->getLine());
}
}
