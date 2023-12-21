<?hh

/*
 * Test for the datefmt_get_timetype  function
 */


function ut_main()
:mixed{
    $timetype_arr = vec[
        IntlDateFormatter::FULL,
        IntlDateFormatter::LONG,
        IntlDateFormatter::MEDIUM,
        IntlDateFormatter::SHORT,
        IntlDateFormatter::NONE
    ];

    $res_str = '';

    foreach( $timetype_arr as $timetype_entry )
    {
        $res_str .= "\nCreating IntlDateFormatter with time_type = $timetype_entry";
        $fmt = ut_datefmt_create( "de-DE",  IntlDateFormatter::SHORT, $timetype_entry ,'America/Los_Angeles', IntlDateFormatter::GREGORIAN  );
        $time_type = ut_datefmt_get_timetype( $fmt);
        $res_str .= "\nAfter call to get_timetype :  timetype= $time_type";
        $res_str .= "\n";
    }

    return $res_str;

}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    // Run the test
    ut_run();
}
