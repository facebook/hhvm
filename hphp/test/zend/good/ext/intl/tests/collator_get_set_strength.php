<?hh

/*
 * Try to set/get collation strength.
 */

/*
 * Set given collation strength, then get it back
 * and check if it's the same.
 */
function check_set_strength( $coll, $val )
:mixed{
    ut_coll_set_strength( $coll, $val );
    $new_val = ut_coll_get_strength( $coll );
    return ( $new_val == $val ? "ok" : "failed" ) . "\n";
}

function ut_main()
:mixed{
    $res = '';
    $coll = ut_coll_create( 'en_US' );

    $res .= check_set_strength( $coll, Collator::PRIMARY );
    $res .= check_set_strength( $coll, Collator::SECONDARY );
    $res .= check_set_strength( $coll, Collator::TERTIARY );

    return $res;
}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    ut_run();
}
