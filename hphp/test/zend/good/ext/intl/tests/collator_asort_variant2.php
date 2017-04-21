<?php

/*
 * Sort associative arrays using various locales.
 */


$test_num = 1;

/*
 * Sort various arrays in specified locale.
 */
function sort_arrays( $locale, $test_arrays, $sort_flag = Collator::SORT_REGULAR )
{
    $res_str = '';

    $coll = ut_coll_create( $locale );

    foreach( $test_arrays as $test_array )
    {
        // Try to sort test data.
        $res_val = ut_coll_asort( $coll, $test_array, $sort_flag );

        // Return output data.
        $res_dump = "\n" . dump( $test_array ) .
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

/*
 * Test main function.
 */
function ut_main()
{
    global $test_num;
    $test_num = 1;
    $res_str = '';

    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = array(
        array( 'd' => 'y'  ,
               'c' => 'i'  ,
               'a' => 'k'  ),

        array( 'a' => 'a'  ,
               'b' => 'aaa',
               'c' => 'aa' ),

        array( 'a'  => 'a' ,
               'aaa'=> 'a' ,
               'aa' => 'a' ),

        array( '1' => 'abc',
               '5' => '!'  ,
               '2' => null ,
               '7' => ''   ),

        array( '1' => '100',
               '2' => '25' ,
               '3' => '36' ),

        array( '1' => 5    ,
               '2' => '30' ,
               '3' => 2    )
    );

    $res_str .= sort_arrays( 'en_US', $test_params );

    // Sort an array in SORT_STRING mode using en_US locale.
    $test_params = array(
        array( '1' => '100',
               '2' => '25' ,
               '3' => '36' ),

        array( '1' => 5    ,
               '2' => '30' ,
               '3' => 2    ),

        array( '1' => 'd'  ,
               '2' => ''   ,
               '3' => ' a' ),

        array( '1' => 'y'  ,
               '2' => 'k'  ,
               '3' => 'i'  )
    );

    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );

    // Sort a non-ASCII array using ru_RU locale.
    $test_params = array(
        array( 'п' => 'у',
               'б' => 'в',
               'е' => 'а' ),

        array( '1' => 'п',
               '4' => '',
               '7' => 'd',
               '2' => 'пп' )
    );

    $res_str .= sort_arrays( 'ru_RU', $test_params );


    // Sort an array using Lithuanian locale.
    $test_params = array(
        array( 'd' => 'y',
               'c' => 'i',
               'a' => 'k' )
    );

    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str . "\n";
}

include_once( 'ut_common.inc' );
ut_run();
?>