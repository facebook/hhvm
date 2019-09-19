<?hh
/* Prototype  : void restore_error_handler(void)
 * Description: Restores the previously defined error handler function
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

function myErrorHandler($errno, $errstr, $errfile, $errline) {
    return true;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing restore_error_handler() : error bug #46196 ***\n";

var_dump( set_error_handler( fun('myErrorHandler') ) );
var_dump( restore_error_handler() );
var_dump( set_error_handler( fun('myErrorHandler') ) );

echo "===DONE===\n";
}
