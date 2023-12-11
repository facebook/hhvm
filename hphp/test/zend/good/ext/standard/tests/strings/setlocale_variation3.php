<?hh
/* Prototype  : string setlocale (int $category , string $locale [,string $..] )
 *            : string setlocale(int $category , array $locale);  
 * Description: Sets locale information.Returns the new current locale , or FALSE
 *              if locale functionality is not implemented in this platform. 
 * Source code: ext/standard/string.c
*/

/* If locale is "0", the locale setting is not affected, only the current setting is returned */
<<__EntryPoint>> function main(): void {
echo "*** Testing setlocale() : usage variations - setting system locale = 0 ***\n";
$locale_info_before = vec[];
$locale_info_after = vec[];

//initially giving the locale
setlocale(LC_ALL,"en_US.utf8");

echo "Locale info, before setting the locale\n";
//returns current locale,before executing setlocale().
$locale_info_before = localeconv();

var_dump($locale_info_before);

//Testing setlocale()  by giving locale = 0
echo "Setting system locale, category = LC_ALL and locale = 0\n";
setlocale(LC_ALL, 0);

echo "Locale info, after setting the locale\n";
//returns current locale,after executing setlocale().
$locale_info_after = localeconv();

var_dump($locale_info_after);

echo "Checking locale in the system, Expected : no change in the existing locale\n";
echo "Test ";
if($locale_info_before ==  $locale_info_after){
  echo "PASSED.";
} else {
  echo "FAILED.";
}

echo "\nDone\n";
}
