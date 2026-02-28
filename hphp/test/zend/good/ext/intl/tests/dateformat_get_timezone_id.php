<?hh

/*
 * Test for the datefmt_get_timezone_id  function
 */


function ut_main()
:mixed{
    $timezone_id_arr = vec[
        'America/New_York',
        'US/Pacific',
        'US/Central'
    ];

    $res_str = '';

    foreach( $timezone_id_arr as $timezone_id_entry )
    {
        $res_str .= "\nCreating IntlDateFormatter with timezone_id = $timezone_id_entry";
        $fmt = ut_datefmt_create( "de-DE",  IntlDateFormatter::SHORT, IntlDateFormatter::SHORT, $timezone_id_entry , IntlDateFormatter::GREGORIAN  );
        $timezone_id = ut_datefmt_get_timezone_id( $fmt);
        $res_str .= "\nAfter call to get_timezone_id :  timezone_id= $timezone_id";
        $res_str .= "\n";
    }

    return $res_str;

}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    // Run the test
    ut_run();
}
