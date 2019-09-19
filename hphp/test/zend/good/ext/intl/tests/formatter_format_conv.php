<?hh

function ut_main()
{
    $fmt = ut_nfmt_create( 'en_US', NumberFormatter::DECIMAL );
    $number = 1234567.891234567890000;

    $str_res = ut_nfmt_format ($fmt, $number, NumberFormatter::TYPE_INT32)."\n";
    return $str_res;
}

include_once( 'ut_common.inc' );
<<__EntryPoint>> function main_entry(): void {
// Run the test
ut_run();
}
