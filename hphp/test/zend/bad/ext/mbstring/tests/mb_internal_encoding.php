<?php
// TODO: 
//$debug = true;
ini_set('include_path', dirname(__FILE__));
include_once('common.inc');

// EUC-JP
$r = mb_internal_encoding('EUC-JP');
($r === TRUE) ? print "OK_EUC-JP_SET\n" : print "NG_EUC-JP_SET\n";
$enc = mb_internal_encoding();
print "$enc\n";

// UTF-8
$r = mb_internal_encoding('UTF-8');
($r === TRUE) ? print "OK_UTF-8_SET\n" : print "NG_UTF-8_SET\n";
$enc = mb_internal_encoding();
print "$enc\n";

// ASCII
$r = mb_internal_encoding('ASCII');
($r === TRUE) ? print "OK_ASCII_SET\n" : print "NG_ASCII_SET\n";
$enc = mb_internal_encoding();
print "$enc\n";

// Invalid Parameter
print "== INVALID PARAMETER ==\n";

// Note: Other than string type, PHP raises Warning
$r = mb_internal_encoding('BAD');
($r === FALSE) ? print "OK_BAD_SET\n" : print "NG_BAD_SET\n";
$enc = mb_internal_encoding();
print "$enc\n";

$r = mb_internal_encoding($t_ary);
($r === FALSE) ? print "OK_BAD_ARY_SET\n" : print "NG_BAD_ARY_SET\n";
$enc = mb_internal_encoding();
print "$enc\n";

$r = mb_internal_encoding($t_obj);
($r === FALSE) ? print "OK_BAD_OBJ_SET\n" : print "NG_BAD_OBJ_SET\n";
$enc = mb_internal_encoding();
print "$enc\n";

?>

