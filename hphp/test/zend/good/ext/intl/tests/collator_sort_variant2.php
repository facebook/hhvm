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
    $test_params = vec[
        vec[ 'abc', 'abd', 'aaa' ],
        vec[ 'm'  , '1'  , '_'   ],
        vec[ 'a'  , 'aaa', 'aa'  ],
        vec[ 'ba' , 'b'  , 'ab'  ],
        vec[ 'e'  , 'c'  , 'a'   ],
        vec[ '100', '25' , '36'  ],
        vec[ 'd'  , ''   , ' a'  ],
        vec[ 'd ' , 'f ' , ' a'  ],
        vec[ 'y'  , 'k'  , 'i' ]
    ];

    $res_str .= sort_arrays( 'en_US', $test_params );

    $test_params = vec[
        vec[ '100', '25' , '36'  ],
        vec[ 5    , '30' , 2     ],
        vec[ 'd'  , ''   , ' a'  ],
        vec[ 'y'  , 'k'  , 'i' ]
    ];

    // Sort in en_US locale with SORT_STRING flag
    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );


    // Sort a non-ASCII array using ru_RU locale.
    $test_params = vec[
        vec[ "\xd0\xb0\xd0\xb1\xd0\xb3", "\xd0\xb0\xd0\xb1\xd0\xb2", "\xd0\xb0\xd0\xb0\xd0\xb0", 'abc' ],
        vec[ "\xd0\xb0\xd0\xb0", "\xd0\xb0\xd0\xb0\xd0\xb0" , "\xd0\xb0" ]
    ];

    $res_str .= sort_arrays( 'ru_RU', $test_params );

    // Sort an array using Lithuanian locale.
    $test_params = vec[
        vec[ 'y'  , 'k'  , 'i' ]
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
