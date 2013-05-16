<?php

/*
 * Try creating collator with different locales
 * with Procedural and Object methods.
 */

function ut_main()
{
    $res_str = '';

    $locales = array(
        'EN-US-ODESSA',
        'UK_UA_ODESSA',
        'uk-ua_CALIFORNIA@currency=;currency=GRN',
        '',
        'root',
        'uk@currency=EURO',
	'1234567891113151719212325272931333537394143454749515357596163656769717375777981838587899193959799'
    );

    foreach( $locales as $locale )
    {
        // Create Collator with the current locale.
        $coll = ut_coll_create( $locale );
        if( !is_object($coll) )
        {
            $res_str .= "Error creating collator with '$locale' locale: " .
                 intl_get_error_message() . "\n";
            continue;
        }

        // Get the requested, valid and actual locales.
        $vloc = ut_coll_get_locale( $coll, Locale::VALID_LOCALE );
        $aloc = ut_coll_get_locale( $coll, Locale::ACTUAL_LOCALE );

        // Show them.
        $res_str .= "Locale: '$locale'\n" .
            "  ULOC_REQUESTED_LOCALE = '$locale'\n" .
            "  ULOC_VALID_LOCALE     = '$vloc'\n" .
            "  ULOC_ACTUAL_LOCALE    = '$aloc'\n";
    }

    return $res_str;
}

include_once( 'ut_common.inc' );
ut_run();

?>