<?hh
//Tests that calling is_callable() on an illegal array just returns false,
//without an array->string conversion notice

function myErrorHandler($errno, $errstr, $errfile, $errline)
:mixed{
        echo "Notice: $errstr\n";
}
<<__EntryPoint>> function main(): void {
set_error_handler(myErrorHandler<>);
$name = null;
var_dump( is_callable_with_name(vec[1,2,3], true, inout $name ) );
echo $name . "\n";
}
