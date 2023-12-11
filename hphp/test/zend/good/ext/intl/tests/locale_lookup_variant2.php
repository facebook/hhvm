<?hh

/*
 * Try parsing different Locales
 * with Procedural and Object methods.
 */

function ut_main()
:mixed{
    $loc_ranges = vec[
        'de-de',
        'sl_IT',
        'sl_IT_Nedis',
        'jbo',
        'art-lojban'
    ];

    $lang_tags = vec[
        'de-DEVA',
        'de-DE-1996',
        'de-DE',
        'zh_Hans',
        'de-CH-1996',
        'sl_IT',
        'sl_IT_nedis-a-kirti-x-xyz',
        'sl_IT_rozaj',
        'sl_IT_NEDIS_ROJAZ_1901',
        'i-enochian',
        'sgn-CH-de',
        'art-lojban',
        'i-lux',
        'art-lojban',
        'jbo',
        'en_sl_IT',
        'zh-Hant-CN-x-prv1-prv2'
    ];


    $res_str = '';
    $isCanonical = false;

    foreach($loc_ranges as $loc_range){
            $res_str .="--------------\n";
            $result= ut_loc_locale_lookup( $lang_tags , $loc_range,$isCanonical,"en_US");
            $comma_arr =implode(",",$lang_tags);
            $res_str .= "loc_range:$loc_range \nlang_tags: $comma_arr\n";
            $res_str .= "\nlookup result:$result\n";
//canonicalized version
            $result= ut_loc_locale_lookup( $lang_tags , $loc_range,!($isCanonical),"en_US");
            $can_loc_range = ut_loc_canonicalize($loc_range);
            $res_str .= "Canonical lookup result:$result\n";

    }

    $res_str .= "\n";
    return $res_str;

}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    ut_run();
}
