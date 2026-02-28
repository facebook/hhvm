<?hh

/*
 * Retreive error code.
 */

/*
 * Check if error code equals to expected one.
 */
function check_rc( $rc, $expected )
:mixed{
    return ( $rc === $expected ? "ok" : "failed" ) . "\n";
}

function ut_main()
:mixed{
    $res = '';
    $coll = ut_coll_create( 'ru_RU' );

    // Try specifying a correct attribute.
    ut_coll_get_attribute( $coll, Collator::NORMALIZATION_MODE );
    $status = ut_coll_get_error_code( $coll );
    $res .= check_rc( $status, U_ZERO_ERROR );

    // Try specifying an incorrect attribute.
    ut_coll_get_attribute( $coll, 12345 );
    $status = ut_coll_get_error_code( $coll );
    $res .= check_rc( $status, U_ILLEGAL_ARGUMENT_ERROR );

    return $res;
}
<<__EntryPoint>>
function main_entry(): void {
  include_once( 'ut_common.inc' );

  // Suppress warning messages.
  error_reporting( E_ERROR );

  ut_run();
}
