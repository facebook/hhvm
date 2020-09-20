<?hh

/*
 * Compare various string pairs using various locales.
 */

include_once( 'ut_common.inc' );

/*
 * Converts comparison result to a character.
 */
function cmp_to_char( $comp_res )
{
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
{
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
{
    $res_str = '';

    // Compare strings using en_US locale.
    $test_params = varray[
        varray[ 'abc', 'abc' ],
        varray[ 'Abc', 'abc' ],
        varray[ 'a'  , 'abc' ],
        varray[ 'a'  , ''    ],
        varray[ ''  , ''     ],
        varray[ 'a'  , 'b'   ],
        varray[ 'ab'  , 'b'  ],
        varray[ 'ab'  , 'a'  ],
        varray[ 123  , 'abc' ],
        varray[ 'ac' , null  ],
        varray[ '.'  , '.'   ],
        // Try to compare long strings.
        varray[ 'abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcde',
               'abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdea'],
        varray[ null , null  ]
    ];

    $res_str .= compare_pairs( 'en_US', $test_params );


    // Compare strings using ru_RU locale.
    $test_params = varray[
        varray[ 'а',   'б' ],
        varray[ 'а',   'аа' ],
        varray[ 'аб', 'ба' ],
        varray[ 'а',   ',' ],
        varray[ 'а',   'b' ],
        varray[ 'а',   'bb' ],
        varray[ 'а',   'ab' ],
        varray[ 'а',   null ]
    ];

    $res_str .= compare_pairs( 'ru_RU', $test_params );


    // Compare strings using lt_LT locale.
    $test_params = varray[
        varray[ 'y', 'k' ]
    ];

    $res_str .= compare_pairs( 'lt_LT', $test_params );

    return $res_str;
}
<<__EntryPoint>>
function main_entry(): void {
  ut_run();
}
