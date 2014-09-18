<?php

ini_set("intl.error_level", E_WARNING);
ini_set("error_reporting", ~E_DEPRECATED);

/*
 * Test for the datefmt_set_timezone_id  function
 */


function ut_main()
{
	$timezone_id_arr = array (
		'America/New_York',
		'America/Los_Angeles',
		'America/Chicago',
		'CN'
	);
	$timestamp_entry = 0;

	$res_str = '';

	$fmt = ut_datefmt_create( "en_US",  IntlDateFormatter::FULL, IntlDateFormatter::FULL, 'US/Pacific' , IntlDateFormatter::GREGORIAN  );
	$timezone_id = ut_datefmt_get_timezone_id( $fmt );
	$res_str .= "\nAfter creation of the dateformatter :  timezone_id= $timezone_id\n";

	foreach( $timezone_id_arr as $timezone_id_entry )
	{

		$res_str .= "-----------";
		$res_str .= "\nTrying to set timezone_id= $timezone_id_entry";
		ut_datefmt_set_timezone_id( $fmt , $timezone_id_entry );
		$timezone_id = ut_datefmt_get_timezone_id( $fmt );
		$res_str .= "\nAfter call to set_timezone_id :  timezone_id= $timezone_id";
		$formatted = ut_datefmt_format( $fmt, 0);
		$res_str .= "\nFormatting timestamp=0 resulted in  $formatted";
		$formatted = ut_datefmt_format( $fmt, 3600);
		$res_str .= "\nFormatting timestamp=3600 resulted in  $formatted";
		$res_str .= "\n";

	}

	return $res_str;

}

include_once( 'ut_common.inc' );

// Run the test
ut_run();
?>
