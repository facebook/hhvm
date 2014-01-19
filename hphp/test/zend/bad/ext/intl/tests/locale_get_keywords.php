<?php

/*
 * Try getting the keywords for different locales
 * with Procedural and Object methods.
 */

function ut_main()
{
    $res_str = '';

    $locales = array(
		"de_DE@currency=EUR;collation=PHONEBOOK",
        'uk-ua_CALIFORNIA@currency=GRN'
	);

    $locales = array(
	'de_DE@currency=EUR;collation=PHONEBOOK',
        'root',
        'uk@currency=EURO',
        'Hindi',
//Simple language subtag
        'de',
        'fr',
        'ja',
        'i-enochian', //(example of a grandfathered tag)
//Language subtag plus Script subtag:	
        'zh-Hant',
        'zh-Hans',
        'sr-Cyrl',
        'sr-Latn',
//Language-Script-Region
        'zh-Hans-CN',
        'sr-Latn-CS',
//Language-Variant
        'sl-rozaj',
        'sl-nedis',
//Language-Region-Variant
        'de-CH-1901',
        'sl-IT-nedis',
//Language-Script-Region-Variant
        'sl-Latn-IT-nedis',
//Language-Region:
        'de-DE',
        'en-US',
        'es-419',
//Private use subtags:
        'de-CH-x-phonebk',
        'az-Arab-x-AZE-derbend',
//Extended language subtags 
        'zh-min',
        'zh-min-nan-Hant-CN',
//Private use registry values
        'x-whatever',
        'qaa-Qaaa-QM-x-southern',
        'sr-Latn-QM',
        'sr-Qaaa-CS',
/*Tags that use extensions (examples ONLY: extensions MUST be defined
   by revision or update to this document or by RFC): */
        'en-US-u-islamCal',
        'zh-CN-a-myExt-x-private',
        'en-a-myExt-b-another',
//Some Invalid Tags:
        'de-419-DE',
        'a-DE',
        'ar-a-aaa-b-bbb-a-ccc'
    );

    $res_str = '';

    foreach( $locales as $locale )
    {
        $keywords_arr = ut_loc_get_keywords( $locale);
        $res_str .= "$locale: ";
		if( $keywords_arr){
			foreach( $keywords_arr as $key => $value){
        			$res_str .= "Key is $key and Value is $value \n";
			}
		}
		else{
			$res_str .= "No keywords found.";
		}
        $res_str .= "\n";
    }

    $res_str .= "\n";
    return $res_str;

}

include_once( 'ut_common.inc' );
ut_run();

?>