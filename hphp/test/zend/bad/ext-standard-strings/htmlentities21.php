<?php
$tests = array(
    0x00, //C0
    0x01,
    0x09,
    0x0A,
    0x0B,
    0x0C,
    0x0D,
    0x0E,
    0x1F,
    0x20, //allowed always
    0x7F, //DEL
    0x80, //C1
    0x9F,
    0xA0, //allowed always
);

function test($flag, $charset) {
    global $tests;
    $i = -1;
    error_reporting(-1 & ~E_STRICT);
    foreach ($tests as $test) {
        $test = chr($test);
        $i++;
        $a = htmlentities($test, $flag | ENT_DISALLOWED, $charset);
        $b = htmlspecialchars($test, $flag | ENT_DISALLOWED, $charset);
        if ($a == "" && $b == "") { echo sprintf("%05X", $tests[$i]), ": INVALID SEQUENCE\n"; continue; }
        echo sprintf("%05X", $tests[$i]), ": ", bin2hex($a), " ", bin2hex($b), "\n";
    }
    error_reporting(-1);
}

echo "*** Testing HTML 4.01/Windows-1251 ***\n";

test(ENT_HTML401, "Windows-1251");

echo "\n*** Testing XHTML 1.0/Windows-1251 ***\n";

test(ENT_XHTML, "Windows-1251");

echo "\n*** Testing HTML 5/Windows-1251 ***\n";

test(ENT_HTML5, "Windows-1251");

echo "\n*** Testing XML 1.0/Windows-1251 ***\n";

test(ENT_XML1, "Windows-1251");

echo "\n*** Testing HTML 4.01/SJIS ***\n";

test(ENT_HTML401, "SJIS");

echo "\n*** Testing XHTML 1.0/SJIS ***\n";

test(ENT_XHTML, "SJIS");

echo "\n*** Testing HTML 5/SJIS ***\n";

test(ENT_HTML5, "SJIS");

echo "\n*** Testing XML 1.0/SJIS ***\n";

test(ENT_XML1, "SJIS");


?>