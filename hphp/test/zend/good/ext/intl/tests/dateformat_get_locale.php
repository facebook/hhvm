<?hh

/*
 * Test for the datefmt_get_locale  function
 */


function ut_main()
:mixed{
    $locale_arr = varray [
        'de-DE',
        'sl-IT-nedis',
        'en_UK',
        'hi'
    ];

    $res_str = '';

    foreach( $locale_arr as $locale_entry )
    {
        $res_str .= "\nCreating IntlDateFormatter with locale = $locale_entry";
        $fmt = ut_datefmt_create( $locale_entry , IntlDateFormatter::SHORT,IntlDateFormatter::SHORT,'America/Los_Angeles', IntlDateFormatter::GREGORIAN  );
        $locale = ut_datefmt_get_locale( $fmt , 1);
        $res_str .= "\nAfter call to get_locale :  locale= $locale";
        $res_str .= "\n";
    }
    $badvals = vec[100, -1, 4294901761];
    foreach($badvals as $badval) {
        if(ut_datefmt_get_locale($fmt, $badval)) {
            $res_str .= "datefmt_get_locale should return false for bad argument $badval\n";
        }
    }

    return $res_str;

}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    // Run the test
    ut_run();
}
