<?php
$string		= str_repeat("1234567890 X ", 10);
$break		= str_repeat("a-very-long-break-string-to-clobber-the-heap", 8);
$linelength	= 10;

echo "Length of original string:  ".strlen($string)."\n";
echo "Length of break string:     ".strlen($break)."\n";

var_dump(wordwrap($string, $linelength, $break, 1));
?>