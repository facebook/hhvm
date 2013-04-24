<?php
$arr = array(
0x20AC => array(0x80, "EURO SIGN"),
//0x81	      	#UNDEFINED
0x201A => array(0x82, "SINGLE LOW-9 QUOTATION MARK"),
0x0192 => array(0x83, "LATIN SMALL LETTER F WITH HOOK"),
0x201E => array(0x84, "DOUBLE LOW-9 QUOTATION MARK"),
0x2026 => array(0x85, "HORIZONTAL ELLIPSIS"),
0x2020 => array(0x86, "DAGGER"),
0x2021 => array(0x87, "DOUBLE DAGGER"),
0x02C6 => array(0x88, "MODIFIER LETTER CIRCUMFLEX ACCENT"),
0x2030 => array(0x89, "PER MILLE SIGN"),
0x0160 => array(0x8A, "LATIN CAPITAL LETTER S WITH CARON"),
0x2039 => array(0x8B, "SINGLE LEFT-POINTING ANGLE QUOTATION MARK"),
0x0152 => array(0x8C, "LATIN CAPITAL LIGATURE OE"),
//0x8D	      	#UNDEFINED
0x017D => array(0x8E, "LATIN CAPITAL LETTER Z WITH CARON"),
//0x8F	      	#UNDEFINED
//0x90	      	#UNDEFINED
0x2018 => array(0x91, "LEFT SINGLE QUOTATION MARK"),
0x2019 => array(0x92, "RIGHT SINGLE QUOTATION MARK"),
0x201C => array(0x93, "LEFT DOUBLE QUOTATION MARK"),
0x201D => array(0x94, "RIGHT DOUBLE QUOTATION MARK"),
0x2022 => array(0x95, "BULLET"),
0x2013 => array(0x96, "EN DASH"),
0x2014 => array(0x97, "EM DASH"),
0x02DC => array(0x98, "SMALL TILDE"),
0x2122 => array(0x99, "TRADE MARK SIGN"),
0x0161 => array(0x9A, "LATIN SMALL LETTER S WITH CARON"),
0x203A => array(0x9B, "SINGLE RIGHT-POINTING ANGLE QUOTATION MARK"),
0x0153 => array(0x9C, "LATIN SMALL LIGATURE OE"),
//0x9D	      	#UNDEFINED
0x017E => array(0x9E, "LATIN SMALL LETTER Z WITH CARON"),
0x0178 => array(0x9F, "LATIN CAPITAL LETTER Y WITH DIAERESIS"),
);

$res = html_entity_decode("&#x81;", ENT_QUOTES, 'WINDOWS-1252');
echo "Special test for &#x81; (shouldn't decode):\n";
echo $res,"\n\n";

$res = html_entity_decode("&#x8D;", ENT_QUOTES, 'WINDOWS-1252');
echo "Special test for &#x8D; (shouldn't decode):\n";
echo $res,"\n\n";

$res = html_entity_decode("&#x8F;", ENT_QUOTES, 'WINDOWS-1252');
echo "Special test for &#x8F; (shouldn't decode):\n";
echo $res,"\n\n";

$res = html_entity_decode("&#x90;", ENT_QUOTES, 'WINDOWS-1252');
echo "Special test for &#x90; (shouldn't decode):\n";
echo $res,"\n\n";

$res = html_entity_decode("&#x9D;", ENT_QUOTES, 'WINDOWS-1252');
echo "Special test for &#x9D; (shouldn't decode):\n";
echo $res,"\n\n";

foreach ($arr as $u => $v) {
    $ent = sprintf("&#x%X;", $u);
    $res = html_entity_decode($ent, ENT_QUOTES, 'WINDOWS-1252');
    $d = unpack("H*", $res);
    echo sprintf("%s: %s => %s\n", $v[1], $ent, $d[1]);
    
    $ent = sprintf("&#x%X;", $v[0]);
    $res = html_entity_decode($ent, ENT_QUOTES, 'WINDOWS-1252');
    if ($res[0] != "&" || $res[1] != "#")
        $res = unpack("H*", $res)[1];
    echo sprintf("%s => %s\n\n", $ent, $res);
}