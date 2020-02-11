<?hh

/*
 * Get locale.
 */

function ut_main()
{
    $locales = varray[
        'en_UK',
        'en_US@California',
        'fr_CA',
    ];

    $loc_types = darray[
        Locale::ACTUAL_LOCALE    => 'actual',
        Locale::VALID_LOCALE     => 'valid',
    ];

    $res_str = '';

    foreach( $locales as $locale )
    {
        $fmt = ut_nfmt_create( $locale, NumberFormatter::DECIMAL );
        $res_str .= "$locale: ";
        foreach( $loc_types as $loc_type => $loc_type_name )
            $res_str .= sprintf( " %s=%s",
            $loc_type_name,
            dump( ut_nfmt_get_locale( $fmt, $loc_type ) ) );
        $res_str .= "\n";
    }

    return $res_str;
}

include_once( 'ut_common.inc' );
<<__EntryPoint>> function main_entry(): void {
// Run the test
ut_run();
}
