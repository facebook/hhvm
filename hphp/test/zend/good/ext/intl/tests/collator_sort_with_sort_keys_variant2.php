<?hh

/*
 * Sort arrays in the given list using specified locale.
 */
function sort_arrays( $locale, $arrays )
:mixed{
    $res_str = '';

    $coll = ut_coll_create( $locale );

    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort_with_sort_keys( $coll, inout $array );

        // Concatenate the sorted array and function result
        // with output string.
        $res_dump = "\n" . dump( $array ) .
                    "\n Result: " . dump( $res_val );


        // Preppend test signature to output string
        $md5 = md5( $res_dump );


        $res_str .= "\n\n".
                    "Test ".ZendGoodExtIntlTestsCollatorSortWithSortKeysVariant2::$test_num.".$md5:" .
                    $res_dump;
        ++ZendGoodExtIntlTestsCollatorSortWithSortKeysVariant2::$test_num;
    }

    return $res_str;
}


function ut_main()
:mixed{

    ZendGoodExtIntlTestsCollatorSortWithSortKeysVariant2::$test_num = 1;
    $res_str = '';

    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = vec[
        vec[ 'abc', 'abd', 'aaa' ],
        vec[ 'm'  , '1'  , '_'   ],
        vec[ 'a'  , 'aaa', 'aa'  ],
        vec[ 'ba' , 'b'  , 'ab'  ],
        vec[ 'e'  , 'c'  , 'a'   ],
        vec[ 'd'  , ''   , ' a'  ],
        vec[ 'd ' , 'f ' , ' a'  ],
        vec[ 'a'  , null , '3'   ],
        vec[ 'y'  , 'i'  , 'k'   ]
    ];

    $res_str .= sort_arrays( 'en_US', $test_params );

    // Sort a non-ASCII array using ru_RU locale.
    $test_params = vec[
        vec[ "\xd0\xb0\xd0\xb1\xd0\xb3", "\xd0\xb0\xd0\xb1\xd0\xb2", "\xd0\xb0\xd0\xb0\xd0\xb0", 'abc' ],
        vec[ "\xd0\xb0\xd0\xb0", "\xd0\xb0\xd0\xb0\xd0\xb0", "\xd0\xb0" ]
    ];

    $res_str .= sort_arrays( 'ru_RU', $test_params );

    // Array with data for sorting.
    $test_params = vec[
        vec[ 'y'  , 'i'  , 'k'   ]
    ];

    // Sort an array using Lithuanian locale.
    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str . "\n";
}

abstract final class ZendGoodExtIntlTestsCollatorSortWithSortKeysVariant2 {
  public static $test_num;
}
<<__EntryPoint>>
function entrypoint_collator_sort_with_sort_keys_variant2(): void {

  /*
   * Sort arrays using various locales.
   */


  ZendGoodExtIntlTestsCollatorSortWithSortKeysVariant2::$test_num = 1;

  include_once( 'ut_common.inc' );
  ut_run();
}
