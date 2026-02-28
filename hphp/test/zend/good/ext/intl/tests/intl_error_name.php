<?hh

/*
 * Check getting error string by integer error code.
 */


function check( $err_code )
:mixed{
    echo intl_error_name( $err_code ) . "\n";
}
<<__EntryPoint>> function main(): void {
check( U_ZERO_ERROR );
check( U_ILLEGAL_ARGUMENT_ERROR );
check( U_USING_FALLBACK_WARNING );
}
