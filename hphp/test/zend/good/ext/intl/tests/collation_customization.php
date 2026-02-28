<?hh

/*
 * Check effects of changing misc collattion options.
 */


function cmp_array( inout $coll, $a )
:mixed{
    $res = '';
    $prev = null;
    foreach( $a as $i )
    {
        if( is_null( $prev ) )
            $res .= "$i";
        else
        {
            $eqrc = ut_coll_compare( $coll, $prev, $i );
            $eq = $eqrc < 0 ? "<" : ( $eqrc > 0 ? ">" : "=" );
            $res .= " $eq $i";
        }

        $prev = $i;
    }
    $res .= "\n";

    return $res;
}

function check_alternate_handling( inout $coll )
:mixed{
    $res = '';

    ut_coll_set_strength( $coll, Collator::TERTIARY );
    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::NON_IGNORABLE );

    $res .= cmp_array( inout $coll, vec[ 'di Silva', 'Di Silva', 'diSilva', 'U.S.A.', 'USA' ] );

    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::SHIFTED );

    $res .= cmp_array( inout $coll, vec[ 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ] );

    ut_coll_set_strength( $coll, Collator::QUATERNARY );

    $res .= cmp_array( inout $coll, vec[ 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ] );
    $res .= "\n";

    return $res;
}

function ut_main()
:mixed{
    $coll = ut_coll_create( 'en_US' );

    return
        check_alternate_handling( inout $coll );
}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    ut_run();
}
