<?hh

trait Test {
  public function __construct() {
 }
  public function func() :mixed{
 }
}

<<__EntryPoint>>
function main_1998() :mixed{
$rconstr = new ReflectionMethod('Test::__construct');
$rfunc = new ReflectionMethod('Test::func');
var_dump($rconstr->isConstructor());
var_dump($rfunc->isConstructor());
}
