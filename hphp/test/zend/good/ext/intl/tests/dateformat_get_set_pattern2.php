<?hh

/*
 * Test for the datefmt_get_pattern & datefmt_set_pattern function
 */


function ut_main()
:mixed{
        $pattern_arr = vec[
                'DD-MM-YYYY hh:mm:ss',
        'yyyy-DDD.hh:mm:ss z',
                "yyyy/MM/dd",
                "yyyyMMdd"
        ];

        $res_str = '';

        $start_pattern = 'dd-MM-YY';
        $res_str .= "\nCreating IntlDateFormatter with pattern = $start_pattern ";
        //$fmt = ut_datefmt_create( "en-US",  IntlDateFormatter::SHORT, IntlDateFormatter::SHORT , 'America/New_York', IntlDateFormatter::GREGORIAN , $start_pattern );
        $fmt = ut_datefmt_create( "en-US",  IntlDateFormatter::FULL, IntlDateFormatter::FULL, 'America/New_York', IntlDateFormatter::GREGORIAN , $start_pattern );
        $pattern = ut_datefmt_get_pattern( $fmt);
        $res_str .= "\nAfter call to get_pattern :  pattern= $pattern";
    $formatted = ut_datefmt_format($fmt,0);
    $res_str .= "\nResult of formatting timestamp=0 is :  \n$formatted";


        foreach( $pattern_arr as $pattern_entry )
        {
                $res_str .= "\n-------------------";
                $res_str .= "\nSetting IntlDateFormatter with pattern = $pattern_entry ";
                ut_datefmt_set_pattern( $fmt , $pattern_entry );
                $pattern = ut_datefmt_get_pattern( $fmt);
                $res_str .= "\nAfter call to get_pattern :  pattern= $pattern";
        $formatted = ut_datefmt_format($fmt,0);
                $res_str .= "\nResult of formatting timestamp=0 with the new pattern is :  \n$formatted";
                $res_str .= "\n";

        }

        return $res_str;

}

<<__EntryPoint>> function main_entry(): void {
        include_once( 'ut_common.inc' );
        // Run the test
        ut_run();
}
