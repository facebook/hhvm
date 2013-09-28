<?php
function codepoint_to_utf8($k) {
	if ($k < 0x80) {
		$retval = pack('C', $k);
	} else if ($k < 0x800) {
		$retval = pack('C2', 
            0xc0 | ($k >> 6),
            0x80 | ($k & 0x3f));
	} else if ($k < 0x10000) {
        $retval = pack('C3',
            0xe0 | ($k >> 12),
            0x80 | (($k >> 6) & 0x3f),
            0x80 | ($k & 0x3f));
	} else {
        $retval = pack('C4',
            0xf0 | ($k >> 18),
            0x80 | (($k >> 12) & 0x3f),
            0x80 | (($k >> 6) & 0x3f),
            0x80 | ($k & 0x3f));
	}
	return $retval;
}

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
    0xD7FF, //surrogates
    0xD800,
    0xDFFF,
    0xE000, //allowed always
    0xFFFE, //nonchar
    0xFFFF,
    0xFDCF, //allowed always
    0xFDD0, //nonchar
    0xFDEF,
    0xFDF0, //allowed always
    0x2FFFE, //nonchar
    0x2FFFF,
);
$tests2 = array_map('codepoint_to_utf8', $tests);

$subchr = codepoint_to_utf8(0xFFFD);

function test($flag) {
    global $tests, $tests2;
    $i = -1;
    foreach ($tests2 as $test) {
        $i++;
        $a = htmlentities($test, $flag | ENT_DISALLOWED, "UTF-8");
        $b = htmlspecialchars($test, $flag | ENT_DISALLOWED, "UTF-8");
        if ($a == "" && $b == "") { echo sprintf("%05X", $tests[$i]), ": INVALID SEQUENCE\n"; continue; }
        echo sprintf("%05X", $tests[$i]), ": ", bin2hex($a), " ", bin2hex($b), "\n";
    }
}

echo "*** Testing HTML 4.01 ***\n";

test(ENT_HTML401);

echo "\n*** Testing XHTML 1.0 ***\n";

test(ENT_XHTML);

echo "\n*** Testing HTML 5 ***\n";

test(ENT_HTML5);

echo "\n*** Testing XML 1.0 ***\n";

test(ENT_XML1);

?>