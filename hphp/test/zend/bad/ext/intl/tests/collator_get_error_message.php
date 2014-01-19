<?php

/*
 * Retreive error message.
 */


function ut_main()
{
    $res = '';
    $coll = ut_coll_create( 'ru_RU' );

    // Try specifying a correct attribute.
    ut_coll_get_attribute( $coll, Collator::NORMALIZATION_MODE );
    $status = ut_coll_get_error_message( $coll );
    $res .= $status . "\n";

    // Try specifying an incorrect attribute.
    ut_coll_get_attribute( $coll, 12345 );
    $status = ut_coll_get_error_message( $coll );
    $res .= $status . "\n";

    return $res;
}

include_once( 'ut_common.inc' );
ut_run();
?>