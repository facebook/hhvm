<?hh

/*
 * Sort various arrays in specified locale.
 */
function sort_arrays( $locale, $test_arrays, $sort_flag = Collator::SORT_REGULAR )
:mixed{
    $res_str = '';

    $coll = ut_coll_create( $locale );

    foreach( $test_arrays as $test_array )
    {
        // Try to sort test data.
        $res_val = ut_coll_asort( $coll, inout $test_array, $sort_flag );

        // Return output data.
        $res_dump = "\n" . dump( $test_array ) .
                    "\n Result: " . dump( $res_val );

		// Preppend test signature to output string
        $md5 = md5( $res_dump );


        $res_str .= "\n\n".
                    "Test ".ZendGoodExtIntlTestsCollatorAsortVariant2::$test_num.".$md5:" .
                    $res_dump;
        ++ZendGoodExtIntlTestsCollatorAsortVariant2::$test_num;
    }

    return $res_str;
}

/*
 * Test main function.
 */
function ut_main()
:mixed{

    ZendGoodExtIntlTestsCollatorAsortVariant2::$test_num = 1;
    $res_str = '';

    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = vec[
        dict[ 'd' => 'y'  ,
               'c' => 'i'  ,
               'a' => 'k'  ],

        dict[ 'a' => 'a'  ,
               'b' => 'aaa',
               'c' => 'aa' ],

        dict[ 'a'  => 'a' ,
               'aaa'=> 'a' ,
               'aa' => 'a' ],

        dict[ '1' => 'abc',
               '5' => '!'  ,
               '7' => ''   ],

        dict[ '1' => '100',
               '2' => '25' ,
               '3' => '36' ],
    ];

    $res_str .= sort_arrays( 'en_US', $test_params );

    // Sort an array in SORT_STRING mode using en_US locale.
    $test_params = vec[
        dict[ '1' => '100',
               '2' => '25' ,
               '3' => '36' ],

        dict[ '1' => 'd'  ,
               '2' => ''   ,
               '3' => ' a' ],

        dict[ '1' => 'y'  ,
               '2' => 'k'  ,
               '3' => 'i'  ]
    ];

    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );

    // Sort a non-ASCII array using ru_RU locale.
    $test_params = vec[
        dict[ 'п' => 'у',
               'б' => 'в',
               'е' => 'а' ],

        dict[ '1' => 'п',
               '4' => '',
               '7' => 'd',
               '2' => 'пп' ]
    ];

    $res_str .= sort_arrays( 'ru_RU', $test_params );


    // Sort an array using Lithuanian locale.
    $test_params = vec[
        dict[ 'd' => 'y',
               'c' => 'i',
               'a' => 'k' ]
    ];

    $res_str .= sort_arrays( 'lt_LT', $test_params );

    return $res_str . "\n";
}

abstract final class ZendGoodExtIntlTestsCollatorAsortVariant2 {
  public static $test_num;
}
<<__EntryPoint>>
function entrypoint_collator_asort_variant2(): void {

  /*
   * Sort associative arrays using various locales.
   */


  ZendGoodExtIntlTestsCollatorAsortVariant2::$test_num = 1;

  include_once( 'ut_common.inc' );
  ut_run();
}
