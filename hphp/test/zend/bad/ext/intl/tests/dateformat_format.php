<?php

//ini_set("intl.error_level", E_WARNING);

/*
 * Test for the datefmt_format  function
 */


function ut_main()
{
	$timezone = 'GMT-10:00';

	$locale_arr = array (
		'en_US'
	);
	
	$datetype_arr = array (
                IntlDateFormatter::FULL,
                IntlDateFormatter::LONG,
                IntlDateFormatter::MEDIUM,
                IntlDateFormatter::SHORT,
                IntlDateFormatter::NONE
        );

        $res_str = '';


	$time_arr = array (
		0,
		-1200000,
		1200000,
		2200000000.0,
		-2200000000.0,
		90099999,
		3600,
		-3600
	);

	$localtime_arr1 = array (
	    'tm_sec' => 24 ,
	    'tm_min' => 3,
	    'tm_hour' => 19,
	    'tm_mday' => 3,
	    'tm_mon' => 3,
	    'tm_year' => 105,
	);
	$localtime_arr2 = array (
	    'tm_sec' => 21,
	    'tm_min' => 5,
	    'tm_hour' => 7,
	    'tm_mday' => 13,
	    'tm_mon' => 4,
	    'tm_year' => 205,
	);
	$localtime_arr3 = array (
            'tm_sec' => 11,
            'tm_min' => 13,
            'tm_hour' => 0,
            'tm_mday' => 17,
            'tm_mon' => 11,
            'tm_year' => -5
        );

	$localtime_arr = array (
		$localtime_arr1,
		$localtime_arr2,
		$localtime_arr3
	);
	
	$d1 = new DateTime("2010-01-01 01:02:03", new DateTimeZone("UTC"));
	$d2 = new DateTime("2000-12-31 03:04:05", new DateTimeZone("UTC"));
	$d2->setTimezone(new DateTimeZone("PDT"));
	$dates = array(
		$d1, 
		$d2,
		new StdClass(),
	);

	//Test format with input as a timestamp : integer
	foreach( $time_arr as $timestamp_entry){
		$res_str .= "\n------------\n";
		$res_str .= "\nInput timestamp is : $timestamp_entry";
		$res_str .= "\n------------\n";
		foreach( $locale_arr as $locale_entry ){
			foreach( $datetype_arr as $datetype_entry )
	{
		$res_str .= "\nIntlDateFormatter locale= $locale_entry ,datetype = $datetype_entry ,timetype =$datetype_entry ";
		$fmt = ut_datefmt_create( $locale_entry , $datetype_entry ,$datetype_entry, $timezone, IntlDateFormatter::GREGORIAN);
		$formatted = ut_datefmt_format( $fmt , $timestamp_entry);
		$res_str .= "\nFormatted timestamp is : $formatted";
	}
	}
	}

	//Test format with input as a localtime :array
	foreach( $localtime_arr as $localtime_entry){
		$res_str .= "\n------------\n";
		$res_str .= "\nInput localtime is : ";
		foreach( $localtime_entry as $key => $value){
                    $res_str .= "$key : '$value' , ";
		}

		$res_str .= "\n------------\n";
		foreach( $locale_arr as $locale_entry ){
			foreach( $datetype_arr as $datetype_entry )
	{
		$res_str .= "\nIntlDateFormatter locale= $locale_entry ,datetype = $datetype_entry ,timetype =$datetype_entry ";
		$fmt = ut_datefmt_create( $locale_entry , $datetype_entry ,$datetype_entry, $timezone, IntlDateFormatter::GREGORIAN );
		$formatted1 = ut_datefmt_format( $fmt , $localtime_entry);
		if( intl_get_error_code() == U_ZERO_ERROR){
			$res_str .= "\nFormatted localtime_array is : $formatted1";
		}else{
			$res_str .= "\nError while formatting as: '".intl_get_error_message()."'";
		}
	}
	}
	}

	foreach($dates as $date_entry) {
		foreach( $locale_arr as $locale_entry ){
			foreach( $datetype_arr as $datetype_entry ) {
				$res_str .= "\n------------";
				$res_str .= "\nDate is: ".var_export($date_entry, true);
				$res_str .= "\n------------";
				
				$fmt = ut_datefmt_create( $locale_entry , $datetype_entry ,$datetype_entry, $timezone, IntlDateFormatter::GREGORIAN );
				$formatted1 = ut_datefmt_format( $fmt , $date_entry);
				if( intl_get_error_code() == U_ZERO_ERROR){
					$res_str .= "\nFormatted DateTime is : $formatted1";
				}else{
					$res_str .= "\nError while formatting as: '".intl_get_error_message()."'";
				}
			}
		}
	}

	return $res_str;

}

include_once( 'ut_common.inc' );

// Run the test
ut_run();
?>