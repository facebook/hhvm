<?php
$tests = array(
    "&#0;", //C0
    "&#1;",
    "&#x09;",
    "&#x0A;",
    "&#x0B;",
    "&#x0C;",
    "&#x0D;", //note that HTML5 is unique in that it forbids this entity, but allows a literal U+0D
    "&#x0E;",
    "&#x1F;",
    "&#x20;", //allowed always
    "&#x7F;", //DEL
    "&#x80;", //C1
    "&#x9F;",
    "&#xA0;", //allowed always
    "&#xD7FF;", //surrogates
    "&#xD800;",
    "&#xDFFF;",
    "&#xE000;", //allowed always
    "&#xFFFE;", //nonchar
    "&#xFFFF;",
    "&#xFDCF;", //allowed always
    "&#xFDD0;", //nonchar
    "&#xFDEF;",
    "&#xFDF0;", //allowed always
    "&#x2FFFE;", //nonchar
    "&#x2FFFF;",
    "&#x110000;", //bad reference
);

function test($flag, $flag2=ENT_DISALLOWED, $charset="UTF-8") {
    global $tests;
    $i = -1;
    error_reporting(-1 & ~E_STRICT);
    foreach ($tests as $test) {
        $i++;
        $a = htmlentities($test, $flag | $flag2, $charset, FALSE);
        $b = htmlspecialchars($test, $flag | $flag2, $charset, FALSE);
        
        if ($a == $b)
            echo sprintf("%s\t%s", $test, $a==$test?"NOT CHANGED":"CHANGED"), "\n";
        else
            echo sprintf("%s\tCHANGED (%s, %s)", $test, $a, $b), "\n";
    }
    error_reporting(-1);
}

echo "*** Testing HTML 4.01 ***\n";

test(ENT_HTML401);

echo "\n*** Testing XHTML 1.0 ***\n";

test(ENT_XHTML);

echo "\n*** Testing HTML 5 ***\n";

test(ENT_HTML5);

echo "\n*** Testing XML 1.0 ***\n";

test(ENT_XML1);

echo "\n*** Testing 5 without the flag ***\n";

test(ENT_HTML5, 0);

echo "\n*** Testing HTML 5 with another single-byte encoding ***\n";

test(ENT_HTML5, ENT_DISALLOWED, "Windows-1251");

echo "\n*** Testing HTML 5 with another multibyte-byte encoding ***\n";

test(ENT_HTML5, ENT_DISALLOWED, "SJIS");

?>