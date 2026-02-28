<?hh

function test():mixed{
    trigger_error("error", E_USER_ERROR);
}

function errorHandler($errno, $errstr, $errfile, $errline) :mixed{
    throw new Exception();
}
<<__EntryPoint>> function main(): void {
set_error_handler(errorHandler<>, E_USER_ERROR);

try{
    test();
}catch(Exception $e){
}

restore_error_handler();

echo "ok";
}
