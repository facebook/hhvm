<?php
/* Prototype  : string setlocale (int $category , string $locale [,string $..] )
              : string setlocale(int $category , array $locale);  
 * Description: Sets locale information.Returns the new current locale , or FALSE if locale functionality is not implemented in this platform. 
 * Source code: ext/standard/string.c
*/

echo "*** Testing setlocale() : error conditions ***\n";

// Zero argument
echo "\n-- Testing setlocale() function with Zero arguments --";
var_dump( setlocale());

// One argument
echo "\n-- Testing setlocale() function with One argument, 'category' = LC_ALL --";
var_dump( setlocale(LC_ALL) );

echo "\n-- Testing setlocale() function with invalid locale array, 'category' = LC_ALL --\n";
//Invalid array of locales
$invalid_locales = array("en_US.invalid", "en_AU.invalid", "ko_KR.invalid");
var_dump( setlocale(LC_ALL,$invalid_locales) );

echo "\n-- Testing setlocale() function with invalid multiple locales, 'category' = LC_ALL --\n";
//Invalid array of locales
var_dump( setlocale(LC_ALL,"en_US.invalid", "en_AU.invalid", "ko_KR.invalid") );

echo "\n-- Testing setlocale() function with invalid category --\n";
//invalid $category
$invalid_category = "TEST";
var_dump( setlocale($invalid_category,"en_US.utf8") );
 
echo "\nDone";
?>