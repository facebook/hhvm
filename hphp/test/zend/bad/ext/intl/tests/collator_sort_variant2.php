<?php

/*
 * Sort arrays using various locales.
 */


$test_num = 1;

/*
 * Sort arrays in the given list using specified locale.
 */
function sort_arrays( $locale, $arrays, $sort_flag = Collator::SORT_REGULAR )
{
    $res_str = '';

    $coll = ut_coll_create( $locale );

    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort( $coll, $array, $sort_flag );

        // Concatenate the sorted array and function result
        // with output string.
        $res_dump = "\n" . dump( $array ) .
                    "\n Result: " . dump( $res_val );

		// Preppend test signature to output string
        $md5 = md5( $res_dump );

        global $test_num;
        
        $res_str .= "\n\n".
                    "Test $test_num.$md5:" .
                    $res_dump;
        ++$test_num;
    }

    return $res_str;
}

function ut_main()
{
    global $test_num;
    $test_num = 1;
    $res_str = '';

    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = array(
        array( 'abc', 'abd', 'aaa' ),
        array( 'm'  , '1'  , '_'   ),
        array( 'a'  , 'aaa', 'aa'  ),
        array( 'ba' , 'b'  , 'ab'  ),
        array( 'e'  , 'c'  , 'a'   ),
        array( '100', '25' , '36'  ),
        array( 5    , '30' , 2     ),
        array( 'd'  , ''   , ' a'  ),
        array( 'd ' , 'f ' , ' a'  ),
        array( 'a'  , null , '3'   ),
        array( 'y'  , 'k'  , 'i' )
    );

    $res_str .= sort_arrays( 'en_US', $test_params );

    $test_params = array(
        array( '100', '25' , '36'  ),
        array( 5    , '30' , 2     ),
        array( 'd'  , ''   , ' a'  ),
        array( 'y'  , 'k'  , 'i' )
    );

    // Sort in en_US locale with SORT_STRING flag
    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );


    // Sort a non-ASCII array using ru_RU locale.
    $test_params = array(
        array( 'абг', 'абв', 'ааа', 'abc' ),
        array( 'аа', 'ааа' , 'а' )
    );

    $res_str .= sort_arrays( 'ru_RU', $test_params );

    // Sort an array using Lithuanian locale.
    $test_params = array(
        array( 'y'  , 'k'  , 'i' )
    );

    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str;
}

include_once( 'ut_common.inc' );
ut_run();
?>