<?hh


function foobar($errno, $errstr, $errfile, $errline) { }

abstract final class ZendGoodZendTestsBug46106 {
  public static $foo;
}

function test($x) {
    try { $x->invokeArgs(varray[0]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
<<__EntryPoint>> function main(): void {
ZendGoodZendTestsBug46106::$foo = varray[1];

set_error_handler(fun('foobar'));

$x = new ReflectionFunction('str_pad');
test($x);

echo "DONE";
}
