<?hh

/*
 * Currency parsing.
 */

function ut_main()
:mixed{
    $res_str = '';

    $fmt = ut_nfmt_create( "en_US", NumberFormatter::CURRENCY );
    $pos = 0;
    $currency = '';
    $num = ut_nfmt_parse_currency( $fmt, '$9,988,776.65', inout $currency, inout $pos );
    $num__str = (string)($num);
    $res_str .= "$num__str $currency\n";

    $fmt = ut_nfmt_create( "en_US", NumberFormatter::CURRENCY );
    $pos = 1;
    $currency = '';
    $num = ut_nfmt_parse_currency( $fmt, ' $123.45', inout $currency, inout $pos );
    $num__str = (string)($num);
    $res_str .=  "$num__str $currency\n";

    return $res_str;
}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    ut_run();
}
