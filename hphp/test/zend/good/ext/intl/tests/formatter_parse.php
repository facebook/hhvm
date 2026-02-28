<?hh

/*
 * Number parsing.
 */


function ut_main()
:mixed{
    $res_str = '';

    $pos = null;
    // Test parsing float number.
    $fmt = ut_nfmt_create( "en_US", NumberFormatter::DECIMAL );
    $res_str .= (string)(ut_nfmt_parse( $fmt, "123E-3", NumberFormatter::TYPE_DOUBLE, inout $pos )) . "\n";

    // Test parsing float number as integer.
    $fmt = ut_nfmt_create( "en_US", NumberFormatter::DECIMAL );
    $pos = null;
    $res_str .= ut_nfmt_parse( $fmt, "1.23", NumberFormatter::TYPE_INT32, inout $pos ) . "\n";

    // Test specifying non-zero parsing start position.
    $fmt = ut_nfmt_create( "en_US", NumberFormatter::DECIMAL );
    $pos = 2;
    $res_str .= (string)(ut_nfmt_parse( $fmt, "0.123 here", NumberFormatter::TYPE_DOUBLE, inout $pos )) . "\n";
    $res_str .= "$pos\n";

    return $res_str;
}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    ut_run();
}
