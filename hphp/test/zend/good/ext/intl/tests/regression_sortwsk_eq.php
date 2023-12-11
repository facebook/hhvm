<?hh
/*
 * Test sorting strings that have different length but otherwise equal.
 */

function sort_using_locale( $locale, $test_array )
:mixed{
    $coll = ut_coll_create( $locale );

    // Sort array.
    ut_coll_sort_with_sort_keys( $coll, inout $test_array );

    // And return the sorted array.
    return dump( $test_array ) . "\n";
}

function ut_main()
:mixed{
    $res_str = '';

    // Define a couple of arrays.
    // Each array contains equal strings that differ only in their length.
    $a1 = vec[ 'aa', 'aaa', 'a' ];
    $a2 = vec[ 'пп', 'ппп', 'п' ];

    // Sort them.
    $res_str .= sort_using_locale( 'en_US', $a1 );
    $res_str .= sort_using_locale( 'ru_RU', $a2 );

    return $res_str;
}
<<__EntryPoint>>
function main_entry(): void {

  require_once( 'ut_common.inc' );
  ut_run();
}
