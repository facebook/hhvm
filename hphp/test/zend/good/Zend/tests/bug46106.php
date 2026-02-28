<?hh


function foobar($errno, $errstr, $errfile, $errline) :mixed{ }

abstract final class ZendGoodZendTestsBug46106 {
  public static $foo;
}

function test($x) :mixed{
    try { $x->invokeArgs(vec[0]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
<<__EntryPoint>> function main(): void {
ZendGoodZendTestsBug46106::$foo = vec[1];

set_error_handler(foobar<>);

$x = new ReflectionFunction('str_pad');
test($x);

echo "DONE";
}
