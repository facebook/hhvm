<?hh

function exception_error_handler() {
        throw new Exception();
}
<<__EntryPoint>> function main(): void {
set_error_handler(fun("exception_error_handler"));
try {
   $undefined->undefined();
} catch(Exception $e) {
    echo "Exception is thrown";
}
}
