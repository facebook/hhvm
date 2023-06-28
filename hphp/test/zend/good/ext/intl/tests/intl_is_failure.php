<?hh
/*
 * Check determining failure error codes.
 */


function check( $err_code )
:mixed{
    var_export( intl_is_failure( $err_code ) );
    echo "\n";
}
<<__EntryPoint>> function main(): void {
check( U_ZERO_ERROR );
check( U_USING_FALLBACK_WARNING );
check( U_ILLEGAL_ARGUMENT_ERROR );
}
