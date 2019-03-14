<?php

function ut_run($mainFunc) {
    $GLOBALS['oo-mode'] = true;
    $oo_result = $mainFunc();
    $GLOBALS['oo-mode'] = false;
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
function dump($val) {
    return var_export( $val, true );
}
function ut_coll_create( $locale )
{
    return $GLOBALS['oo-mode'] ? Collator::create( $locale ) : collator_create( $locale );
}
function ut_coll_compare( $coll, $str1, $str2 )
{
    return $GLOBALS['oo-mode'] ?
      $coll->compare( $str1, $str2 ) : collator_compare( $coll, $str1, $str2 );
}
function ut_coll_sort_with_sort_keys( $coll, &$arr )
{
    return $GLOBALS['oo-mode'] ?
      $coll->sortWithSortKeys( &$arr ) : collator_sort_with_sort_keys( $coll, &$arr );
}
function ut_coll_sort( $coll, &$arr, $sort_flag = Collator::SORT_REGULAR )
{
    return $GLOBALS['oo-mode'] ?
      $coll->sort( &$arr, $sort_flag ) : collator_sort( $coll, &$arr, $sort_flag );
}
function ut_coll_asort( $coll, &$arr, $sort_flag = Collator::SORT_REGULAR )
{
    return $GLOBALS['oo-mode'] ?
      $coll->asort( &$arr, $sort_flag ) : collator_asort( $coll, &$arr, $sort_flag );
}
function ut_coll_get_locale( $coll, $type )
{
    return $GLOBALS['oo-mode'] ?
      $coll->getLocale( $type ) : collator_get_locale( $coll, $type );
}
function ut_coll_set_strength( $coll, $strength )
{
    return $GLOBALS['oo-mode'] ?
      $coll->setStrength( $strength ) : collator_set_strength( $coll, $strength );
}
function ut_coll_set_attribute( $coll, $attr, $val )
{
    return $GLOBALS['oo-mode'] ?
      $coll->setAttribute( $attr, $val ) : collator_set_attribute( $coll, $attr, $val );
}
function ut_coll_set_default( $coll )
{
    return $GLOBALS['oo-mode'] ? Collator::setDefault( $coll ) : collator_set_default( $coll );
}
function sort_arrays( $locale, $arrays, $sort_flag = Collator::SORT_REGULAR )
{
    $res_str = '';
    $coll = ut_coll_create( $locale );
    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort( $coll, &$array, $sort_flag );
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
{

    ExtCollator1810::$test_num = 1;
    $res_str = '';
    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = array(
        array( 'abc', 'abd', 'aaa' ),
        array( 'm'  , '1'  , '_'   ),
        array( 'a'  , 'aaa', 'aa'  ),
        array( 'ba' , 'b'  , 'ab'  ),
        array( 'e'  , 'c'  , 'a'   ),
        array( '100', '25' , '36'  ), // test 6
        array( 5    , '30' , 2     ),
        array( 'd'  , ''   , ' a'  ),
        array( 'd ' , 'f ' , ' a'  ),
        array( 'a'  , null , '3'   ),
        array( 'y'  , 'k'  , 'i' )
    );
    $res_str .= sort_arrays( 'en_US', $test_params );
    $test_params = array(
        array( '100', '25' , '36'  ),
        array( 5    , '30' , 2     ), // test 13
        array( 'd'  , ''   , ' a'  ),
        array( 'y'  , 'k'  , 'i' )
    );
    // Sort in en_US locale with SORT_STRING flag
    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );
    // Sort a non-ASCII array using ru_RU locale.
    $test_params = array(
        array( 'абг',                'абв',                'ааа',                'абв' ),
        array( 'аа', 'ааа',               'а' )
    );
    $res_str .= sort_arrays( 'ru_RU', $test_params );
    // Sort an array using Lithuanian locale.
    $test_params = array(
        array( 'y'  , 'k'  , 'i' )
    );
    $res_str .= sort_arrays( 'lt_LT', $test_params );
    return $res_str;
}
function ut_main2() {
  $obj = ut_coll_create('en_US');
  $arr0 = array( 100, 25, 36, '30.2', '30.12' );
 // test 6
  $arr1 = array( '100', '25', '36'  );
 // test 6
  $arr2 = array( 11, 5, '2', 64, 17, '30', 10, 2, '54' );
  // strcmp 17 and 30, ret = 1
  // Comparing values 17 and 30, ret = 1
  $arr3 = array( 11, 5, 2, 64, 17, 30, 10, 2, 54 );
  $arrA = $arr0;
  $arrB = $arr0;
  $arrC = $arr0;
  ut_coll_sort($obj, &$arrA, Collator::SORT_REGULAR);
  ut_coll_sort($obj, &$arrB, Collator::SORT_STRING);
  ut_coll_sort($obj, &$arrC, Collator::SORT_NUMERIC);
  var_dump($arrA, $arrB, $arrC);
  $arrA = $arr1;
  $arrB = $arr1;
  $arrC = $arr1;
  ut_coll_sort($obj, &$arrA, Collator::SORT_REGULAR);
  ut_coll_sort($obj, &$arrB, Collator::SORT_STRING);
  ut_coll_sort($obj, &$arrC, Collator::SORT_NUMERIC);
  var_dump($arrA, $arrB, $arrC);
  $arrA = $arr2;
  $arrB = $arr2;
  $arrC = $arr2;
  ut_coll_sort($obj, &$arrA, Collator::SORT_REGULAR);
  ut_coll_sort($obj, &$arrB, Collator::SORT_STRING);
  ut_coll_sort($obj, &$arrC, Collator::SORT_NUMERIC);
  var_dump($arrA, $arrB, $arrC);
  $arrA = $arr3;
  $arrB = $arr3;
  $arrC = $arr3;
  ut_coll_sort($obj, &$arrA, Collator::SORT_REGULAR);
  ut_coll_sort($obj, &$arrB, Collator::SORT_STRING);
  ut_coll_sort($obj, &$arrC, Collator::SORT_NUMERIC);
  var_dump($arrA, $arrB, $arrC);
}
function ut_main3()
{
    $res_str = '';
    $locales = array(
        'EN-US-ODESSA',
        'UK_UA_ODESSA',
        '',
        'root',
        'uk@currency=EURO'
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
        $vloc = ut_coll_get_locale( $coll, 1 ); // was Locale::VALID_LOCALE
        // Show them.
        $res_str .= "Locale: '$locale'\n" .
            "  ULOC_VALID_LOCALE     = '$vloc'\n";
    }
    return $res_str;
}
function test_COW( $locale, $test_array )
{
    $res_str = '';
    $coll = ut_coll_create( $locale );
    // Create two copies of the given array.
    $copy1 = $test_array;
    $copy2 = $test_array;
    // Sort given array and the first copy of it.
    ut_coll_sort( $coll, &$test_array );
    ut_coll_sort( $coll, &$copy1      );
    // Return contents of all the arrays.
    // The second copy should remain unsorted.
    $res_str .= dump( $test_array ) . "\n";
    $res_str .= dump( $copy1 ) . "\n";
    $res_str .= dump( $copy2 ) . "\n";
    return $res_str;
}
function ut_main4()
{
    $res_str = '';
    $a1 = array( 'b', 'a', 'c' );
    $a2 = array( 'б', 'а', 'в' );
    $res_str .= test_COW( 'en_US', $a1 );
    $res_str .= test_COW( 'ru_RU', $a2 );
    return $res_str;
}
function cmp_array( &$coll, $a )
{
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
function check_alternate_handling( &$coll )
{
    $res = '';
    ut_coll_set_strength( $coll, Collator::TERTIARY );
    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::NON_IGNORABLE );
    $res .= cmp_array( &$coll, array( 'di Silva', 'Di Silva', 'diSilva', 'U.S.A.', 'USA' ) );
    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::SHIFTED );
    $res .= cmp_array( &$coll, array( 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ) );
    ut_coll_set_strength( $coll, Collator::QUATERNARY );
    $res .= cmp_array( &$coll, array( 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ) );
    $res .= "\n";
    return $res;
}
function ut_main5()
{
    $coll = ut_coll_create( 'en_US' );
    return
        check_alternate_handling( &$coll );
}
function sort_arrays_with_sort_keys( $locale, $arrays )
{
    $res_str = '';
    $coll = ut_coll_create( $locale );
    foreach( $arrays as $array )
    {
        // Sort array values
        $res_val = ut_coll_sort_with_sort_keys( $coll, &$array );
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
{

    ExtCollator1810::$test_num = 1;
    $res_str = '';
    // Sort an array in SORT_REGULAR mode using en_US locale.
    $test_params = array(
        array( 'abc', 'abd', 'aaa' ),
        array( 'm'  , '1'  , '_'   ),
        array( 'a'  , 'aaa', 'aa'  ),
        array( 'ba' , 'b'  , 'ab'  ),
        array( 'e'  , 'c'  , 'a'   ),
        array( 'd'  , ''   , ' a'  ),
        array( 'd ' , 'f ' , ' a'  ),
        array( 'a'  , null , '3'   ),
        array( 'y'  , 'i'  , 'k'   )
    );
    $res_str .= sort_arrays_with_sort_keys( 'en_US', $test_params );
    // Sort a non-ASCII array using ru_RU locale.
    $test_params = array(
        array( 'абг',                'абв',                'ааа',                'абв' ),
        array( 'аа', 'ааа',               'а' )
    );
    $res_str .= sort_arrays_with_sort_keys( 'ru_RU', $test_params );
    // Array with data for sorting.
    $test_params = array(
        array( 'y'  , 'i'  , 'k'   )
    );
    // Sort an array using Lithuanian locale.
    $res_str .= sort_arrays_with_sort_keys( 'lt_LT', $test_params );
    return $res_str . "\n";
}

<<__EntryPoint>>
function main_1810() {
$test_num = 1;
ut_run('ut_main1');
ut_run('ut_main2');
ut_run('ut_main3');
ut_run('ut_main4');
ut_run('ut_main5');
ut_run('ut_main6');
}

abstract final class ExtCollator1810 {
  public static $test_num;
}
