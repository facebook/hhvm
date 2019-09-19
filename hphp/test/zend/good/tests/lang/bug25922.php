<?hh

function my_error_handler($error, $errmsg='', $errfile='', $errline=0, $errcontext='')
{
    echo "$errmsg\n";
    $errcontext = '';
}

function test()
{
    echo "Undefined index here: '{$data['HTTP_HEADER']}'\n";
}

<<__EntryPoint>> function main(): void {
set_error_handler(fun('my_error_handler'));

test();
}
