<?hh

/*
 * Sort arrays using various locales.
 */


ZendGoodExtIntlTestsCollatorSortWithSortKeys::ZendGoodExtIntlTestsCollatorSortWithSortKeys::$test_num = 1;

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


        $res_str .= "\n\n".
                    "Test ".ZendGoodExtIntlTestsCollatorSortWithSortKeys::$test_num.".$md5:" .
                    $res_dump;
        ++ZendGoodExtIntlTestsCollatorSortWithSortKeys::$test_num;
    }

    return $res_str;
}


function ut_main()
{

    ZendGoodExtIntlTestsCollatorSortWithSortKeys::$test_num = 1;
    $res_str = '';

    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = varray[
        varray[ 'abc', 'abd', 'aaa' ],
        varray[ 'm'  , '1'  , '_'   ],
        varray[ 'a'  , 'aaa', 'aa'  ],
        varray[ 'ba' , 'b'  , 'ab'  ],
        varray[ 'e'  , 'c'  , 'a'   ],
        varray[ 'd'  , ''   , ' a'  ],
        varray[ 'd ' , 'f ' , ' a'  ],
        varray[ 'a'  , null , '3'   ],
        varray[ 'y'  , 'i'  , 'k'   ]
    ];

    $res_str .= sort_arrays( 'en_US', $test_params );

    // Sort a non-ASCII array using ru_RU locale.
    $test_params = varray[
        varray[ 'абг', 'абв', 'ааа', 'abc' ],
        varray[ 'аа', 'ааа', 'а' ]
    ];

    $res_str .= sort_arrays( 'ru_RU', $test_params );

    // Array with data for sorting.
    $test_params = varray[
        varray[ 'y'  , 'i'  , 'k'   ]
    ];

    // Sort an array using Lithuanian locale.
    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str . "\n";
}

include_once( 'ut_common.inc' );
ut_run();

abstract final class ZendGoodExtIntlTestsCollatorSortWithSortKeys {
  public static $test_num;
}
