<?hh

function test(){
    trigger_error("error", E_USER_ERROR);
}

function errorHandler($errno, $errstr, $errfile, $errline) {
    throw new Exception();
}
<<__EntryPoint>> function main(): void {
set_error_handler(fun('errorHandler'), E_USER_ERROR);

try{
    test();
}catch(Exception $e){
}

restore_error_handler();

echo "ok";
}
