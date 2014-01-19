<?php

/*
 * Get sort keys using various locales
 */
function sort_arrays( $locale, $data )
{
    $res_str = '';

    $coll = ut_coll_create( $locale );

	foreach($data as $value) {
		$res_val = ut_coll_get_sort_key( $coll, $value );
		$res_str .= "source: ".$value."\n".
					"key: ".bin2hex($res_val)."\n";
	}

    return $res_str;
}


function ut_main()
{
    $res_str = '';

    // Regular strings keys
    $test_params = array(
		'abc', 'abd', 'aaa',
		'аа', 'а', 'z',
		'', null , '3',
        'y'  , 'i'  , 'k'
    );

    $res_str .= sort_arrays( 'en_US', $test_params );

    // Sort a non-ASCII array using ru_RU locale.
    $test_params = array(
		'абг', 'абв', 'жжж', 'эюя'
    );

    $res_str .= sort_arrays( 'ru_RU', $test_params );

    // Sort an array using Lithuanian locale.
    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str . "\n";
}

include_once( 'ut_common.inc' );
ut_run();
?>