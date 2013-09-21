<?php
/* Prototype  : string htmlspecialchars  ( string $string  [, int $quote_style  [, string $charset  [, bool $double_encode  ]]] )
 * Description: Convert special characters to HTML entities
 * Source code: ext/standard/string.c
*/

echo "*** Testing htmlspecialchars() : basic functionality ***\n";

$s1 = "abc<>\"&\n";
$s2 = "&&abc<>\"&\n";
$s3 = "a>,\<bc<>\"&\n";
$s4 = "a\'\'&bc<>\"&\n";
$s5 = "&amp;&lt;\n";
echo "Basic tests\n";
echo "Test 1: " . htmlspecialchars ($s1);
echo "Test 2: " . htmlspecialchars ($s2);
echo "Test 3: " . htmlspecialchars ($s3);
echo "Test 4: " . htmlspecialchars ($s4);
echo "Test 5: " . htmlspecialchars ($s5);
echo "Test 6: " . htmlspecialchars ($s1,ENT_NOQUOTES);
echo "Test 7: " . htmlspecialchars ($s2,ENT_NOQUOTES);
echo "Test 8: " . htmlspecialchars ($s3,ENT_NOQUOTES);
echo "Test 9: " . htmlspecialchars ($s4,ENT_NOQUOTES);
echo "Test 10: " . htmlspecialchars ($s5,ENT_NOQUOTES);
echo "Test 11: " . htmlspecialchars ($s1,ENT_COMPAT);
echo "Test 12: " . htmlspecialchars ($s2,ENT_COMPAT);
echo "Test 13: " . htmlspecialchars ($s3,ENT_COMPAT);
echo "Test 14: " . htmlspecialchars ($s4,ENT_COMPAT);
echo "Test 15: " . htmlspecialchars ($s5,ENT_COMPAT);
echo "Test 16: " . htmlspecialchars ($s1,ENT_QUOTES);
echo "Test 17: " . htmlspecialchars ($s2,ENT_QUOTES);
echo "Test 18: " . htmlspecialchars ($s3,ENT_QUOTES);
echo "Test 19: " . htmlspecialchars ($s4,ENT_QUOTES);
echo "Test 20: " . htmlspecialchars ($s5,ENT_QUOTES);

echo "\nTry with char set option - specify default ISO-8859-1\n";
echo  "Test 21: " . htmlspecialchars ($s1,ENT_NOQUOTES, "ISO-8859-1");
echo  "Test 22: " . htmlspecialchars ($s2,ENT_COMPAT, "ISO-8859-1");
echo  "Test 23: " . htmlspecialchars ($s3,ENT_QUOTES, "ISO-8859-1");
echo  "Test 24: " . htmlspecialchars ($s5,ENT_QUOTES, "ISO-8859-1");

echo "\nTry with double decode FALSE\n";
$s1 = "&quot;&amp;xyz&gt;abc&quot;\n";
$s2 = "&quot;&amp;123&lt;456&quot;\n";
$s3 = "\"300 < 400\"\n";
echo  "Test 25: " . htmlspecialchars ($s1,ENT_NOQUOTES, "ISO-8859-1", false);
echo  "Test 26: " . htmlspecialchars ($s2,ENT_NOQUOTES, "ISO-8859-1", false);
echo  "Test 27: " . htmlspecialchars ($s3,ENT_NOQUOTES, "ISO-8859-1", false);

echo "\nTry with double decode TRUE\n";
echo  "Test 28: " . htmlspecialchars ($s1, ENT_NOQUOTES, "ISO-8859-1", true);
echo  "Test 29: " . htmlspecialchars ($s2, ENT_NOQUOTES, "ISO-8859-1", true);

?>
===DONE===