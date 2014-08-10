<?php

/*
 * Get/set various number formatting attributes.
 */


function ut_main()
{
    // attr_name => array( attr, value )
    $attributes = array(
        'PARSE_INT_ONLY' => array( NumberFormatter::PARSE_INT_ONLY, 1, 12345.123456 ),
        'GROUPING_USED' => array( NumberFormatter::GROUPING_USED, 0, 12345.123456 ),
        'DECIMAL_ALWAYS_SHOWN' => array( NumberFormatter::DECIMAL_ALWAYS_SHOWN, 1, 12345 ),
        'MAX_INTEGER_DIGITS' => array( NumberFormatter::MAX_INTEGER_DIGITS, 2, 12345.123456 ),
        'MIN_INTEGER_DIGITS' => array( NumberFormatter::MIN_INTEGER_DIGITS, 20, 12345.123456 ),
        'INTEGER_DIGITS' => array( NumberFormatter::INTEGER_DIGITS, 7, 12345.123456 ),
        'MAX_FRACTION_DIGITS' => array( NumberFormatter::MAX_FRACTION_DIGITS, 2, 12345.123456 ),
        'MIN_FRACTION_DIGITS' => array( NumberFormatter::MIN_FRACTION_DIGITS, 20, 12345.123456 ),
        'FRACTION_DIGITS' => array( NumberFormatter::FRACTION_DIGITS, 5, 12345.123456 ),
        'MULTIPLIER' => array( NumberFormatter::MULTIPLIER, 2, 12345.123456 ),
        'GROUPING_SIZE' => array( NumberFormatter::GROUPING_SIZE, 2, 12345.123456 ),
        'ROUNDING_MODE' => array( NumberFormatter::ROUNDING_MODE, 1, 12345.123456 ),
        'ROUNDING_INCREMENT' => array( NumberFormatter::ROUNDING_INCREMENT, (float)2, 12345.123456 ),
        'FORMAT_WIDTH' => array( NumberFormatter::FORMAT_WIDTH, 27, 12345.123456 ),
        'PADDING_POSITION' => array( NumberFormatter::PADDING_POSITION, 2, 12345.123456 ),
        'SECONDARY_GROUPING_SIZE' => array( NumberFormatter::SECONDARY_GROUPING_SIZE, 2, 12345.123456 ),
        'SIGNIFICANT_DIGITS_USED' => array( NumberFormatter::SIGNIFICANT_DIGITS_USED, 1, 12345.123456 ),
        'MIN_SIGNIFICANT_DIGITS' => array( NumberFormatter::MIN_SIGNIFICANT_DIGITS, 3, 1 ),
        'MAX_SIGNIFICANT_DIGITS' => array( NumberFormatter::MAX_SIGNIFICANT_DIGITS, 4, 12345.123456 ),
        // 'LENIENT_PARSE' => array( NumberFormatter::LENIENT_PARSE, 2, 12345.123456 )
    );

    $res_str = '';

    $fmt = ut_nfmt_create( "en_US", NumberFormatter::DECIMAL );

    foreach( $attributes as $attr_name => $args )
    {
        list( $attr, $new_val, $number ) = $args;
        $res_str .= "\nAttribute $attr_name\n";

        // Get original value of the attribute.
        $orig_val = ut_nfmt_get_attribute( $fmt, $attr );

        // Format the number using the original attribute value.
        $rc = ut_nfmt_format( $fmt, $number );

        $ps = ut_nfmt_parse( $fmt, $rc );

        $res_str .= sprintf( "Old attribute value: %s ;  Format result: %s ; Parse result: %s\n",
                             dump( $orig_val ),
                             dump( $rc ),
                             dump( $ps ) );

        // Set new attribute value.
        $rc = ut_nfmt_set_attribute( $fmt, $attr, $new_val );
        if( $rc )
            $res_str .= "Setting attribute: ok\n";
        else
            $res_str .= sprintf( "Setting attribute failed: %s\n", ut_nfmt_get_error_message( $fmt ) );

        // Format the number using the new value.
        $rc = ut_nfmt_format( $fmt, $number );

        // Get current value of the attribute and check if it equals $new_val.
        $attr_val_check = ut_nfmt_get_attribute( $fmt, $attr );
        if( $attr_val_check !== $new_val )
            $res_str .= "ERROR: New $attr_name attribute value has not been set correctly.\n";

        $ps = ut_nfmt_parse( $fmt, $rc );

        $res_str .= sprintf( "New attribute value: %s ;  Format result: %s ; Parse result: %s\n",
                             dump( $new_val ),
                             dump( $rc ),
                             dump( $ps ) );


        // Restore original attribute of the  value
        if( $attr != NumberFormatter::INTEGER_DIGITS && $attr != NumberFormatter::FRACTION_DIGITS
             && $attr != NumberFormatter::FORMAT_WIDTH && $attr != NumberFormatter::SIGNIFICANT_DIGITS_USED )
            ut_nfmt_set_attribute( $fmt, $attr, $orig_val );
    }

    return $res_str;
}

include_once( 'ut_common.inc' );

// Run the test
ut_run();

?>