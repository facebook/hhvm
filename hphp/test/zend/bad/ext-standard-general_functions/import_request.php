<?php
parse_str("a=1&b=heh&c=3&d[]=5&GLOBALS=test&1=hm", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

parse_str("ap=25&bp=test&cp=blah3&dp[]=ar", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);


var_dump(import_request_variables());
var_dump(import_request_variables(""));
var_dump(import_request_variables("", ""));

var_dump(import_request_variables("g", ""));
var_dump($a, $b, $c, $ap);

var_dump(import_request_variables("g", "g_"));
var_dump($g_a, $g_b, $g_c, $g_ap, $g_1);

var_dump(import_request_variables("GP", "i_"));
var_dump($i_a, $i_b, $i_c, $i_ap, $i_bp, $i_cp, $i_dp);

var_dump(import_request_variables("gGg", "r_"));
var_dump($r_a, $r_b, $r_c, $r_ap);

echo "Done\n";
?>