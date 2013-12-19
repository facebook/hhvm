<?php

/*
 * Test for the datefmt_parse  function
 */

putenv('TZ=America/Los_Angeles');

function ut_main()
{
	$locale_arr = array (
		'en_US_CA'
	);

	$datetype_arr = array (
                IntlDateFormatter::FULL,
                IntlDateFormatter::LONG,
                IntlDateFormatter::MEDIUM,
                IntlDateFormatter::SHORT,
                IntlDateFormatter::NONE
        );

        $res_str = '';


	$text_arr = array (
		// Full parsing
		array("Sunday, September 18, 2039 4:06:40 PM PT", IntlDateFormatter::FULL, IntlDateFormatter::FULL),
		array("Wednesday, December 17, 1969 6:40:00 PM PT", IntlDateFormatter::FULL, IntlDateFormatter::FULL),
		array("Thursday, December 18, 1969 8:49:59 PM PST", IntlDateFormatter::FULL, IntlDateFormatter::FULL),
		array("December 18, 1969 8:49:59 AM PST", IntlDateFormatter::LONG, IntlDateFormatter::FULL),
		array("12/18/69 8:49 AM", IntlDateFormatter::SHORT, IntlDateFormatter::SHORT),
		array("19691218 08:49 AM", IntlDateFormatter::SHORT, IntlDateFormatter::SHORT),
		// Partial parsing
		array("Sunday, September 18, 2039 4:06:40 PM PT", IntlDateFormatter::FULL, IntlDateFormatter::NONE),
		array("Sunday, September 18, 2039 4:06:40 PM PT", IntlDateFormatter::FULL, IntlDateFormatter::SHORT),
		array("December 18, 1969 8:49:59 AM PST", IntlDateFormatter::LONG, IntlDateFormatter::NONE),
		array("December 18, 1969 8:49:59 AM PST", IntlDateFormatter::LONG, IntlDateFormatter::SHORT),
		array("12/18/69 8:49 AM", IntlDateFormatter::SHORT, IntlDateFormatter::LONG),
		array("19691218 08:49 AM", IntlDateFormatter::SHORT, IntlDateFormatter::LONG),
	);

	foreach( $text_arr as $text_entry){
		$fmt = ut_datefmt_create( 'en_US_CA', $text_entry[1], $text_entry[2]);
		$parse_pos = 0;
		$parsed = ut_datefmt_parse( $fmt , $text_entry[0] , $parse_pos );

		$res_str .= "\nInput text : {$text_entry[0]} ; DF = {$text_entry[1]}; TF = {$text_entry[2]}";
		if( intl_get_error_code() != U_ZERO_ERROR) {
			$res_str .= "\nError : ".intl_get_error_message();
		}
		$res_str .= "\nParsed: $parsed; parse_pos : $parse_pos\n";
	}

	return $res_str;

}

include_once( 'ut_common.inc' );

// Run the test
ut_run();
?>