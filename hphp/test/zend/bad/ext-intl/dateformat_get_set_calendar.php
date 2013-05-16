<?php

/*
 * Test for the datefmt_get_calendar  and datefmt_set_calendar functions
 */


function ut_main()
{
	$calendar_arr = array (
		IntlDateFormatter::GREGORIAN,
		IntlDateFormatter::TRADITIONAL,
		3
	);
	
	$res_str = '';

	$start_calendar = IntlDateFormatter::GREGORIAN;
	$res_str .= "\nCreating IntlDateFormatter with calendar = $start_calendar";
	$fmt = ut_datefmt_create( "de-DE",  IntlDateFormatter::SHORT, IntlDateFormatter::SHORT ,'America/Los_Angeles', IntlDateFormatter::GREGORIAN);
	$calendar = ut_datefmt_get_calendar( $fmt);
	$res_str .= "\nAfter call to get_calendar :  calendar= $calendar";
	$res_str .= "\n-------------------";

	foreach( $calendar_arr as $calendar_entry )
	{
		$res_str .= "\nSetting IntlDateFormatter with calendar = $calendar_entry";
		ut_datefmt_set_calendar( $fmt, $calendar_entry);
		$calendar = ut_datefmt_get_calendar( $fmt);
		$res_str .= "\nAfter call to get_calendar :  calendar= $calendar";
		$res_str .= "\n-------------------";
	}

	return $res_str;

}

include_once( 'ut_common.inc' );

// Run the test
ut_run();
?>