<?hh

/*
 * Try to set/get a collation attribute.
 */


/*
 * Return current normalication mode.
 */
function check_val( $coll )
:mixed{
    $val = ut_coll_get_attribute( $coll, Collator::NORMALIZATION_MODE );
    return sprintf( "%s\n", ( $val == Collator::OFF ? "off" : "on" ) );
}

function ut_main()
:mixed{
    $res = '';
    $coll = ut_coll_create( 'en_US' );

    ut_coll_set_attribute( $coll, Collator::NORMALIZATION_MODE, Collator::OFF );
    $res .= check_val( $coll );

    ut_coll_set_attribute( $coll, Collator::NORMALIZATION_MODE, Collator::ON );
    $res .= check_val( $coll );

    return $res;
}

<<__EntryPoint>> function main_entry(): void {
    include( 'ut_common.inc' );
    ut_run();
}
