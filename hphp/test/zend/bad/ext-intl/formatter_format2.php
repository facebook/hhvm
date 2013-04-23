<?php

/*
 * Format a number using misc locales/patterns.
 */

/*
 * TODO: doesn't pass on ICU 3.6 because 'ru' and 'de' locales changed
 * currency and percent formatting.
 */

function ut_main()
{
    $styles = array(
        NumberFormatter::PATTERN_DECIMAL => '##.#####################',
        NumberFormatter::DECIMAL => '',
        NumberFormatter::CURRENCY => '',
        NumberFormatter::PERCENT => '',
        NumberFormatter::SCIENTIFIC => '',
        NumberFormatter::SPELLOUT => '@@@@@@@',
        NumberFormatter::ORDINAL => '',
        NumberFormatter::DURATION => '',
        NumberFormatter::PATTERN_RULEBASED => '#####.###',
        1234999, // bad one
    );

   $integer = array(
        NumberFormatter::ORDINAL => '',
        NumberFormatter::DURATION => '',
   );
    $locales = array(
        'en_US',
        'ru_UA',
        'de',
        'fr',
        'en_UK'
    );

    $str_res = '';
    $number = 1234567.891234567890000;

    foreach( $locales as $locale )
    {
        $str_res .= "\nLocale is: $locale\n";
        foreach( $styles as $style => $pattern )
        {
            $fmt = ut_nfmt_create( $locale, $style, $pattern );

			if(!$fmt) {
				$str_res .= "Bad formatter!\n";
				continue;
			}
            $str_res .= dump( isset($integer[$style])?ut_nfmt_format( $fmt, $number, NumberFormatter::TYPE_INT32):ut_nfmt_format( $fmt, $number ) ) . "\n";
        }
    }
    return $str_res;
}

include_once( 'ut_common.inc' );

// Run the test
ut_run();

?>