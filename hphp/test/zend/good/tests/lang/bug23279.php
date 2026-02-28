<?hh

function redirect_on_error($e) :mixed{
    ob_end_clean();
    echo "Goodbye Cruel World\n";
}
<<__EntryPoint>> function main(): void {
ob_start();
set_exception_handler(redirect_on_error<>);
echo "Hello World\n";
throw new Exception;
}
