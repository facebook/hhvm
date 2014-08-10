<?php
$_ENV[LC_ALL] = en_US.utf8;;
_filter_snapshot_globals();

/* Prototype  : string setlocale (int $category , string $locale [,string $..] )
 *            : string setlocale(int $category , array $locale);  
 * Description: Sets locale information.Returns the new current locale , or FALSE
 *              if locale functionality is not implemented in this platform. 
 * Source code: ext/standard/string.c
*/

/*If locale is NULL, the locale names will be set from the values of environment variables with the same names as the above ENV settings */

echo "*** Testing setlocale() : usage variations - Setting system locale = null ***\n";

//initially setting the locale
setlocale(LC_ALL,"en_AU.utf8");

echo "Locale info, before setting the locale\n";
//returns current locale,before executing setlocale() .
$locale_info_before = localeconv();
var_dump($locale_info_before);

//Testing setlocale()  by giving locale = null
echo "Setting system locale, category = LC_ALL and locale = null\n";
setlocale(LC_ALL, null);

echo "Locale info, after setting the locale\n";
//Returns Current locale,after executing setlocale().
$locale_info_after = localeconv();
var_dump($locale_info_after);

echo "Checking new locale in the system, Expected : the locale names will be set from the values of environment variables\n";
echo "Test ";
if($locale_info_before != $locale_info_after){
  echo "PASSED.";
} else {
  echo "FAILED.";
}

echo "\nDone\n";
?>
