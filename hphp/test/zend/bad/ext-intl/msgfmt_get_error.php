<?php

/*
 * Error handling.
 */


function ut_main()
{
    $fmt = ut_msgfmt_create( "en_US", "{0, number} monkeys on {1, number} trees" );
    $num = ut_msgfmt_format( $fmt, array());
    if( $num === false )
        return $fmt->getErrorMessage() . " (" . $fmt->getErrorCode() . ")\n";
    else
        return "Ooops, an error should have occured.";
}

include_once( 'ut_common.inc' );

// Run the test
ut_run();
?>