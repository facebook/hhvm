<?hh

/*
 * Compare various string pairs using various locales.
 */

/*
 * Converts comparison result to a character.
 */
function cmp_to_char( $comp_res )
:mixed{
    switch( $comp_res )
    {
    case 0:            // UCOL_EQUAL
        return '=';
    case 1:            // UCOL_GREATER
        return '>';
    case -1:           // UCOL_LESS
        return '<';
    default:
        return '?';
    }
}

/*
 * Compare string pairs in the given array
 * using specified locale.
 */
function compare_pairs( $locale, $test_array )
:mixed{
    $res_str = '';

    $coll = ut_coll_create( $locale );

    foreach( $test_array as $test_strings )
    {
        list( $str1, $str2 ) = $test_strings;

        // Compare strings.
        $res_val = cmp_to_char( ut_coll_compare( $coll, $str1, $str2 ) );

        // Concatenate result strings.
        $res_str .= dump( $str1 ) .
                    ' ' . $res_val . ' ' .
                    dump( $str2 ) . "\n";
    }

    return $res_str;

}

function ut_main()
:mixed{
    $res_str = '';

    // Compare strings using en_US locale.
    $test_params = vec[
        vec[ 'abc', 'abc' ],
        vec[ 'Abc', 'abc' ],
        vec[ 'a'  , 'abc' ],
        vec[ 'a'  , ''    ],
        vec[ ''  , ''     ],
        vec[ 'a'  , 'b'   ],
        vec[ 'ab'  , 'b'  ],
        vec[ 'ab'  , 'a'  ],
        vec[ 123  , 'abc' ],
        vec[ 'ac' , null  ],
        vec[ '.'  , '.'   ],
        // Try to compare long strings.
        vec[ 'abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcde',
               'abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdea'],
        vec[ null , null  ]
    ];

    $res_str .= compare_pairs( 'en_US', $test_params );


    // Compare strings using ru_RU locale.
    $test_params = vec[
        vec[ 'а',   'б' ],
        vec[ 'а',   'аа' ],
        vec[ 'аб', 'ба' ],
        vec[ 'а',   ',' ],
        vec[ 'а',   'b' ],
        vec[ 'а',   'bb' ],
        vec[ 'а',   'ab' ],
        vec[ 'а',   null ]
    ];

    $res_str .= compare_pairs( 'ru_RU', $test_params );


    // Compare strings using lt_LT locale.
    $test_params = vec[
        vec[ 'y', 'k' ]
    ];

    $res_str .= compare_pairs( 'lt_LT', $test_params );

    return $res_str;
}
<<__EntryPoint>>
function main_entry(): void {
    include_once( 'ut_common.inc' );
    ut_run();
}
