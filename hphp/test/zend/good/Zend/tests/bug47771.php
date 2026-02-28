<?hh
function throw_exc() :mixed{
  throw new Exception('TEST_EXCEPTION');
}

class Test {

  public function __construct() {
    echo 'Constr' ."\n";
  }

}
<<__EntryPoint>> function main(): void {
try {

  $T =new Test(throw_exc());

} catch( Exception $e) {
  echo 'Exception: ' . $e->getMessage() . "\n";
}
}
