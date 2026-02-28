<?hh

function ut_run($mainFunc) :mixed{
    \HH\global_set('oo-mode', true);
    $oo_result = $mainFunc();
    \HH\global_set('oo-mode', false);
    $proc_result = $mainFunc();
    if($proc_result !== $oo_result) {
      echo "ERROR: OO- and procedural APIs produce different results!\n";
      echo "OO API output:\n";
      echo str_repeat( '=', 78 ) . "\n";
      echo $oo_result;
      echo str_repeat( '=', 78 ) . "\n";
      echo "procedural API output:\n";
      echo str_repeat( '=', 78 ) . "\n";
      echo $proc_result;
      echo str_repeat( '=', 78 ) . "\n";
      return;
    }
    echo $oo_result;
}
function dump($val) :mixed{
    return var_export( $val, true );
}
function ut_coll_create( $locale )
:mixed{
    return \HH\global_get('oo-mode') ? Collator::create( $locale ) : collator_create( $locale );
}
function ut_coll_compare( $coll, $str1, $str2 )
:mixed{
    return \HH\global_get('oo-mode') ?
      $coll->compare( $str1, $str2 ) : collator_compare( $coll, $str1, $str2 );
}
function ut_coll_sort_with_sort_keys( $coll, inout $arr )
:mixed{
    return \HH\global_get('oo-mode') ?
      $coll->sortWithSortKeys( inout $arr ) : collator_sort_with_sort_keys( $coll, inout $arr );
}
function ut_coll_sort( $coll, inout $arr, $sort_flag = Collator::SORT_REGULAR )
:mixed{
    return\HH\global_get('oo-mode') ?
      $coll->sort( inout $arr, $sort_flag ) : collator_sort( $coll, inout $arr, $sort_flag );
}
function ut_coll_asort( $coll, inout $arr, $sort_flag = Collator::SORT_REGULAR )
:mixed{
    return \HH\global_get('oo-mode') ?
      $coll->asort( inout $arr, $sort_flag ) : collator_asort( $coll, inout $arr, $sort_flag );
}
function ut_coll_get_locale( $coll, $type )
:mixed{
    return \HH\global_get('oo-mode') ?
      $coll->getLocale( $type ) : collator_get_locale( $coll, $type );
}
function ut_coll_set_strength( $coll, $strength )
:mixed{
    return \HH\global_get('oo-mode') ?
      $coll->setStrength( $strength ) : collator_set_strength( $coll, $strength );
}
function ut_coll_set_attribute( $coll, $attr, $val )
:mixed{
    return \HH\global_get('oo-mode') ?
      $coll->setAttribute( $attr, $val ) : collator_set_attribute( $coll, $attr, $val );
}
function ut_coll_set_default( $coll )
:mixed{
    return \HH\global_get('oo-mode') ? Collator::setDefault( $coll ) : collator_set_default( $coll );
}
function sort_arrays( $locale, $arrays, $sort_flag = Collator::SORT_REGULAR )
:mixed{
    $res_str = '';
    $coll = ut_coll_create( $locale );
    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort( $coll, inout $array, $sort_flag );
        // Concatenate the sorted array and function result
        // with output string.
        $res_dump = "\n" . dump( $array ) .
                    "\n Result: " . dump( $res_val );
        // Preppend test signature to output string
        $md5 = md5( $res_dump );


        $res_str .= "\n\n".
                    "Test ".ExtCollator1810::$test_num.".$md5:" .
                    $res_dump;
        ++ExtCollator1810::$test_num;
    }
    return $res_str;
}
function ut_main1()
:mixed{

    ExtCollator1810::$test_num = 1;
    $res_str = '';
    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = vec[
        vec[ 'abc', 'abd', 'aaa' ],
        vec[ 'm'  , '1'  , '_'   ],
        vec[ 'a'  , 'aaa', 'aa'  ],
        vec[ 'ba' , 'b'  , 'ab'  ],
        vec[ 'e'  , 'c'  , 'a'   ],
        vec[ '100', '25' , '36'  ], // test 6
        vec[ 'd'  , ''   , ' a'  ],
        vec[ 'd ' , 'f ' , ' a'  ],
        vec[ 'y'  , 'k'  , 'i' ]
    ];
    $res_str .= sort_arrays( 'en_US', $test_params );
    $test_params = vec[
        vec[ '100', '25' , '36'  ],
        vec[ 'd'  , ''   , ' a'  ],
        vec[ 'y'  , 'k'  , 'i' ]
    ];
    // Sort in en_US locale with SORT_STRING flag
    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );
    // Sort a non-ASCII array using ru_RU locale.
    $test_params = vec[
        vec[ 'абг',                'абв',                'ааа',                'абв' ],
        vec[ 'аа', 'ааа',               'а' ]
    ];
    $res_str .= sort_arrays( 'ru_RU', $test_params );
    // Sort an array using Lithuanian locale.
    $test_params = vec[
        vec[ 'y'  , 'k'  , 'i' ]
    ];
    $res_str .= sort_arrays( 'lt_LT', $test_params );
    return $res_str;
}
function ut_main3()
:mixed{
    $res_str = '';
    $locales = vec[
        'EN-US-ODESSA',
        'UK_UA_ODESSA',
        '',
        'root',
        'uk@currency=EURO'
    ];
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
        $vloc = ut_coll_get_locale( $coll, 1 ); // was Locale::VALID_LOCALE
        // Show them.
        $res_str .= "Locale: '$locale'\n" .
            "  ULOC_VALID_LOCALE     = '$vloc'\n";
    }
    return $res_str;
}
function test_COW( $locale, $test_array )
:mixed{
    $res_str = '';
    $coll = ut_coll_create( $locale );
    // Create two copies of the given array.
    $copy1 = $test_array;
    $copy2 = $test_array;
    // Sort given array and the first copy of it.
    ut_coll_sort( $coll, inout $test_array );
    ut_coll_sort( $coll, inout $copy1      );
    // Return contents of all the arrays.
    // The second copy should remain unsorted.
    $res_str .= dump( $test_array ) . "\n";
    $res_str .= dump( $copy1 ) . "\n";
    $res_str .= dump( $copy2 ) . "\n";
    return $res_str;
}
function ut_main4()
:mixed{
    $res_str = '';
    $a1 = vec[ 'b', 'a', 'c' ];
    $a2 = vec[ 'б', 'а', 'в' ];
    $res_str .= test_COW( 'en_US', $a1 );
    $res_str .= test_COW( 'ru_RU', $a2 );
    return $res_str;
}
function cmp_array( inout $coll, $a )
:mixed{
    $res = '';
    $prev = null;
    foreach( $a as $i )
    {
        if( is_null( $prev ) )
            $res .= "$i";
        else
        {
            $eqrc = ut_coll_compare( $coll, $prev, $i );
            $eq = $eqrc < 0 ? "<" : ( $eqrc > 0 ? ">" : "=" );
            $res .= " $eq $i";
        }
        $prev = $i;
    }
    $res .= "\n";
    return $res;
}
function check_alternate_handling( inout $coll )
:mixed{
    $res = '';
    ut_coll_set_strength( $coll, Collator::TERTIARY );
    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::NON_IGNORABLE );
    $res .= cmp_array( inout $coll, vec[ 'di Silva', 'Di Silva', 'diSilva', 'U.S.A.', 'USA' ] );
    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::SHIFTED );
    $res .= cmp_array( inout $coll, vec[ 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ] );
    ut_coll_set_strength( $coll, Collator::QUATERNARY );
    $res .= cmp_array( inout $coll, vec[ 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ] );
    $res .= "\n";
    return $res;
}
function ut_main5()
:mixed{
    $coll = ut_coll_create( 'en_US' );
    return
        check_alternate_handling( inout $coll );
}
function sort_arrays_with_sort_keys( $locale, $arrays )
:mixed{
    $res_str = '';
    $coll = ut_coll_create( $locale );
    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort_with_sort_keys( $coll, inout $array );
        // Concatenate the sorted array and function result
        // with output string.
        $res_dump = "\n" . dump( $array ) .
                    "\n Result: " . dump( $res_val );


        // Preppend test signature to output string
        $md5 = md5( $res_dump );

        $res_str .= "\n\n".
                    "Test ".ExtCollator1810::$test_num.".$md5:" .
                    $res_dump;
        ++ExtCollator1810::$test_num;
    }
    return $res_str;
}
function ut_main6()
:mixed{

    ExtCollator1810::$test_num = 1;
    $res_str = '';
    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = vec[
        vec[ 'abc', 'abd', 'aaa' ],
        vec[ 'm'  , '1'  , '_'   ],
        vec[ 'a'  , 'aaa', 'aa'  ],
        vec[ 'ba' , 'b'  , 'ab'  ],
        vec[ 'e'  , 'c'  , 'a'   ],
        vec[ 'd'  , ''   , ' a'  ],
        vec[ 'd ' , 'f ' , ' a'  ],
        vec[ 'y'  , 'i'  , 'k'   ]
    ];
    $res_str .= sort_arrays_with_sort_keys( 'en_US', $test_params );
    // Sort a non-ASCII array using ru_RU locale.
    $test_params = vec[
        vec[ 'абг',                'абв',                'ааа',                'абв' ],
        vec[ 'аа', 'ааа',               'а' ]
    ];
    $res_str .= sort_arrays_with_sort_keys( 'ru_RU', $test_params );
    // Array with data for sorting.
    $test_params = vec[
        vec[ 'y'  , 'i'  , 'k'   ]
    ];
    // Sort an array using Lithuanian locale.
    $res_str .= sort_arrays_with_sort_keys( 'lt_LT', $test_params );
    return $res_str . "\n";
}

<<__EntryPoint>>
function main_1810() :mixed{
$test_num = 1;
ut_run(ut_main1<>);
ut_run(ut_main3<>);
ut_run(ut_main4<>);
ut_run(ut_main5<>);
ut_run(ut_main6<>);
}

abstract final class ExtCollator1810 {
  public static $test_num;
}
