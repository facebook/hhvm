<?php

/*
 * Test for the datefmt_format  function
 */


function ut_main()
{
	$timezone = 'GMT+05:00'; 

	$locale_arr = array (
		'en_US'
	);
	
	$datetype_arr = array (
                IntlDateFormatter::FULL,
                IntlDateFormatter::LONG,
                IntlDateFormatter::MEDIUM
        );

        $res_str = '';


	$time_arr = array (
		0,
		-1200000,
		1200000,
		2200000000,
		-2200000000,
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
	    'tm_mon' => 7,
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

	//Test format and parse with a timestamp : long
	foreach( $time_arr as $timestamp_entry){
		$res_str .= "\n------------\n";
		$res_str .= "\nInput timestamp is : $timestamp_entry";
		$res_str .= "\n------------\n";
		foreach( $locale_arr as $locale_entry ){
			foreach( $datetype_arr as $datetype_entry ) {
				$res_str .= "\nIntlDateFormatter locale= $locale_entry ,datetype = $datetype_entry ,timetype =$datetype_entry ";
				$fmt = ut_datefmt_create( $locale_entry , $datetype_entry ,$datetype_entry,$timezone);
				$formatted = ut_datefmt_format( $fmt , $timestamp_entry);
				$res_str .= "\nFormatted timestamp is : $formatted";
				$parsed = ut_datefmt_parse( $fmt , $formatted);
				if( intl_get_error_code() == U_ZERO_ERROR){
					$res_str .= "\nParsed timestamp is : $parsed";
				}else{
					$res_str .= "\nError while parsing as: '".intl_get_error_message()."'";
				}
			}
		}
	}

	//Test format and parse with a localtime :array
	foreach( $localtime_arr as $localtime_entry){
		$res_str .= "\n------------\n";
		$res_str .= "\nInput localtime is : ";
		foreach( $localtime_entry as $key => $value){
                    $res_str .= "$key : '$value' , ";
		}

		$res_str .= "\n------------\n";
		foreach( $locale_arr as $locale_entry ){
			foreach( $datetype_arr as $datetype_entry ) {
				$res_str .= "\nIntlDateFormatter locale= $locale_entry ,datetype = $datetype_entry ,timetype =$datetype_entry ";
				$fmt = ut_datefmt_create( $locale_entry , $datetype_entry ,$datetype_entry,$timezone);
				$formatted1 = ut_datefmt_format( $fmt , $localtime_entry);
				if( intl_get_error_code() == U_ZERO_ERROR){
					$res_str .= "\nFormatted localtime_array is : $formatted1";
				}else{
					$res_str .= "\nError while formatting as: '".intl_get_error_message()."'";
				}
				//Parsing
				$parsed_arr = ut_datefmt_localtime( $fmt, $formatted1 );

				if( $parsed_arr){
				    $res_str .= "\nParsed array is: ";
				    foreach( $parsed_arr as $key => $value){
					    $res_str .= "$key : '$value' , ";
				    }
				}
/*
				else{
				    //$res_str .= "No values found from LocaleTime parsing.";
				    $res_str .= "\tError : '".intl_get_error_message()."'";
				}
*/
			}
		}
	}

	return $res_str;

}

include_once( 'ut_common.inc' );

// Run the test
ut_run();
?>