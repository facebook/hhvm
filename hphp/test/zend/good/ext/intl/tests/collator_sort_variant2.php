<?hh

/*
 * Sort arrays in the given list using specified locale.
 */
function sort_arrays( $locale, $arrays, $sort_flag = Collator::SORT_REGULAR )
:mixed{
    $res_str = '';

    $coll = ut_coll_create( $locale );

    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort( $coll, inout $array, $sort_flag );

        // Concatenate the sorted array and function result
        // with output string.
        $res_dump = "\n" . dump( $array ) .
                    "\n Result: " . dump( $res_val );

		// Preppend test signature to output string
        $md5 = md5( $res_dump );


        $res_str .= "\n\n".
                    "Test ".ZendGoodExtIntlTestsCollatorSortVariant2::$test_num.".$md5:" .
                    $res_dump;
        ++ZendGoodExtIntlTestsCollatorSortVariant2::$test_num;
    }

    return $res_str;
}

function ut_main()
:mixed{

    ZendGoodExtIntlTestsCollatorSortVariant2::$test_num = 1;
    $res_str = '';

    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = varray[
        varray[ 'abc', 'abd', 'aaa' ],
        varray[ 'm'  , '1'  , '_'   ],
        varray[ 'a'  , 'aaa', 'aa'  ],
        varray[ 'ba' , 'b'  , 'ab'  ],
        varray[ 'e'  , 'c'  , 'a'   ],
        varray[ '100', '25' , '36'  ],
        varray[ 'd'  , ''   , ' a'  ],
        varray[ 'd ' , 'f ' , ' a'  ],
        varray[ 'y'  , 'k'  , 'i' ]
    ];

    $res_str .= sort_arrays( 'en_US', $test_params );

    $test_params = varray[
        varray[ '100', '25' , '36'  ],
        varray[ 5    , '30' , 2     ],
        varray[ 'd'  , ''   , ' a'  ],
        varray[ 'y'  , 'k'  , 'i' ]
    ];

    // Sort in en_US locale with SORT_STRING flag
    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );


    // Sort a non-ASCII array using ru_RU locale.
    $test_params = varray[
        varray[ 'абг', 'абв', 'ааа', 'abc' ],
        varray[ 'аа', 'ааа' , 'а' ]
    ];

    $res_str .= sort_arrays( 'ru_RU', $test_params );

    // Sort an array using Lithuanian locale.
    $test_params = varray[
        varray[ 'y'  , 'k'  , 'i' ]
    ];

    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str;
}

abstract final class ZendGoodExtIntlTestsCollatorSortVariant2 {
  public static $test_num;
}
<<__EntryPoint>>
function entrypoint_collator_sort_variant2(): void {

  /*
   * Sort arrays using various locales.
   */


  ZendGoodExtIntlTestsCollatorSortVariant2::$test_num = 1;

  include_once( 'ut_common.inc' );
  ut_run();
}
