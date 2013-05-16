<?php
/*
 * Check getting global error message.
 */

// Suppress warning messages.
error_reporting( E_ERROR );

if( collator_get_locale() !== false )
    echo "failed\n";
else
    printf( "%s\n", intl_get_error_message() );

?>