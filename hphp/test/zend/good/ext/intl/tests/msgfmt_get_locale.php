<?hh

/*
 * Get locale.
 */

function ut_main()
:mixed{
    $locales = vec[
        'en_UK',
        'en_US@California',
        'uk',
    ];

    $res_str = '';

    foreach( $locales as $locale )
    {
        $fmt = ut_msgfmt_create( $locale, "Test" );
        $res_str .= "$locale: " . dump( ut_msgfmt_get_locale( $fmt ) );
        $res_str .= "\n";
    }

    return $res_str;
}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    // Run the test
    ut_run();
}
