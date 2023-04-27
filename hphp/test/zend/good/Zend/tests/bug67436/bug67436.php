<?hh
<<__EntryPoint>> function main(): void {
set_error_handler(function ($errno, $errstr, $errfile, $errline) {
}, error_reporting());

a::staticTest();

$b = new b();
$b->test();
}
