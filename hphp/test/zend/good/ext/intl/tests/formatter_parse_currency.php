<?hh

/*
 * Currency parsing.
 */

function ut_main()
{
    $res_str = '';

    $fmt = ut_nfmt_create( "en_US", NumberFormatter::CURRENCY );
    $pos = 0;
    $currency = '';
    $num = ut_nfmt_parse_currency( $fmt, '$9,988,776.65', inout $currency, inout $pos );
    $res_str .= "$num $currency\n";

    $fmt = ut_nfmt_create( "en_US", NumberFormatter::CURRENCY );
    $pos = 1;
    $currency = '';
    $num = ut_nfmt_parse_currency( $fmt, ' $123.45', inout $currency, inout $pos );
    $res_str .=  "$num $currency\n";

    return $res_str;
}

include_once( 'ut_common.inc' );
<<__EntryPoint>> function main_entry(): void {
ut_run();
}
