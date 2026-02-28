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
        'art-lojban',
        'sl_IT'
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
        'en_sl_IT'
    ];

    $res_str = '';
    $isCanonical = false;
    foreach($loc_ranges as $loc_range){
        foreach($lang_tags as $lang_tag){
            $res_str .="--------------\n";
            $result= ut_loc_locale_filter_matches( $lang_tag , $loc_range , $isCanonical);
            $res_str .= "loc_range:$loc_range matches lang_tag $lang_tag ? ";
            if( $result){
                $res_str .= "YES\n";
            }else{
                $res_str .= "NO\n";
            }
//canonicalized version
            $result= ut_loc_locale_filter_matches( $lang_tag , $loc_range , !($isCanonical));
            $can_loc_range = ut_loc_canonicalize($loc_range);
            $can_lang_tag = ut_loc_canonicalize($lang_tag);
            $res_str .= "loc_range:$can_loc_range canonically matches lang_tag $can_lang_tag ? ";
            if( $result){
                $res_str .= "YES\n";
            }else{
                $res_str .= "NO\n";
            }
        }
    }

    $res_str .= "\n";
    return $res_str;

}

<<__EntryPoint>> function main_entry(): void {
    include_once( 'ut_common.inc' );
    ut_run();
}
