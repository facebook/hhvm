<?hh

/*
 * Test for the datefmt_get_datetype  function
 */


function ut_main()
:mixed{
    $datetype_arr = vec[
        IntlDateFormatter::FULL,
        IntlDateFormatter::LONG,
        IntlDateFormatter::MEDIUM,
        IntlDateFormatter::SHORT,
        IntlDateFormatter::NONE
    ];

    $res_str = '';

    foreach( $datetype_arr as $datetype_entry )
    {
        $res_str .= "\nCreating IntlDateFormatter with date_type = $datetype_entry";
        $fmt = ut_datefmt_create( "de-DE",  $datetype_entry , IntlDateFormatter::SHORT,'America/Los_Angeles', IntlDateFormatter::GREGORIAN  );
        $date_type = ut_datefmt_get_datetype( $fmt);
        $res_str .= "\nAfter call to get_datetype :  datetype= $date_type";
        $res_str .= "\n";
    }

    return $res_str;

}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    // Run the test
    ut_run();
}
