<?php // vim600: syn=php
/* include('test.inc'); */
//error_reporting(E_ALL);
$utf = implode('', file(dirname(__FILE__).'/Quotes.UTF-8'));

print(iconv("UTF-8", "ISO-8859-1//TRANSLIT", $utf));
print(iconv("UTF-8", "ASCII//TRANSLIT", $utf));
?>