<?php

/*
 * Sort arrays using various locales.
 */


$test_num = 1;

/*
 * Sort arrays in the given list using specified locale.
 */
function sort_arrays( $locale, $arrays )
{
    $res_str = '';

    $coll = ut_coll_create( $locale );

    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort_with_sort_keys( $coll, $array );

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
        array( 'd'  , ''   , ' a'  ),
        array( 'd ' , 'f ' , ' a'  ),
        array( 'a'  , null , '3'   ),
        array( 'y'  , 'i'  , 'k'   )
    );

    $res_str .= sort_arrays( 'en_US', $test_params );

    // Sort a non-ASCII array using ru_RU locale.
    $test_params = array(
        array( 'абг', 'абв', 'ааа', 'abc' ),
        array( 'аа', 'ааа', 'а' )
    );

    $res_str .= sort_arrays( 'ru_RU', $test_params );

    // Array with data for sorting.
    $test_params = array(
        array( 'y'  , 'i'  , 'k'   )
    );

    // Sort an array using Lithuanian locale.
    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str . "\n";
}

include_once( 'ut_common.inc' );
ut_run();
?>