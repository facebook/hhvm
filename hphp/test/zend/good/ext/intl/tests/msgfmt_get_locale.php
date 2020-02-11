<?hh

/*
 * Get locale.
 */

function ut_main()
{
    $locales = varray[
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

include_once( 'ut_common.inc' );
<<__EntryPoint>> function main_entry(): void {
// Run the test
ut_run();
}
