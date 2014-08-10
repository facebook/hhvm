<?php
// TODO: Add more encodings

//$debug=true;
ini_set('include_path','.');
include_once('common.inc');


// Test string
$euc_jp = b'0123����ʸ��������ܸ�Ǥ���EUC-JP��ȤäƤ��ޤ���0123���ܸ�����ݽ�����';

// EUC-JP - With encoding parameter
mb_internal_encoding('UTF-8') or print("mb_internal_encoding() failed\n");

echo  "== POSITIVE OFFSET ==\n";
print  mb_strpos($euc_jp, b'���ܸ�', 0, 'EUC-JP') . "\n";
print  mb_strpos($euc_jp, b'0', 0,     'EUC-JP') . "\n";
print  mb_strpos($euc_jp, 3, 0,       'EUC-JP') . "\n";
print  mb_strpos($euc_jp, 0, 0,       'EUC-JP') . "\n";
print  mb_strpos($euc_jp, b'���ܸ�', 15, 'EUC-JP') . "\n";
print  mb_strpos($euc_jp, b'0', 15,     'EUC-JP') . "\n";
print  mb_strpos($euc_jp, 3, 15,       'EUC-JP') . "\n";
print  mb_strpos($euc_jp, 0, 15,       'EUC-JP') . "\n";

// Negative offset
// Note: PHP Warning - offset is negative.
// Note: For offset(-15). It does not return position of latter string. (ie the same result as -50)
echo "== NEGATIVE OFFSET ==\n";
$r = mb_strpos($euc_jp, b'���ܸ�', -15, 'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";
$r = mb_strpos($euc_jp, b'0', -15,     'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";
$r = mb_strpos($euc_jp, 3, -15,       'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";
$r = mb_strpos($euc_jp, 0, -15,       'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";
$r = mb_strpos($euc_jp, b'���ܸ�', -50, 'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";
$r = mb_strpos($euc_jp, b'0', -50,     'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";
$r = mb_strpos($euc_jp, 3, -50,       'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";
$r = mb_strpos($euc_jp, 0, -50,       'EUC-JP');
($r === FALSE) ? print "OK_NEGATIVE_OFFSET\n" : print "NG_NEGATIVE_OFFSET\n";

// Out of range - should return false
print ("== OUT OF RANGE ==\n");
$r =  mb_strpos($euc_jp, b'���ܸ�', 40, 'EUC-JP');
($r === FALSE) ? print "OK_OUT_RANGE\n"     : print "NG_OUT_RANGE\n";
$r =  mb_strpos($euc_jp, b'0', 40,     'EUC-JP');
($r === FALSE) ? print "OK_OUT_RANGE\n"     : print "NG_OUT_RANGE\n";
$r =  mb_strpos($euc_jp, 3, 40,       'EUC-JP');
($r === FALSE) ? print "OK_OUT_RANGE\n"     : print "NG_OUT_RANGE\n";
$r =   mb_strpos($euc_jp, 0, 40,       'EUC-JP');
($r === FALSE) ? print "OK_OUT_RANGE\n"     : print "NG_OUT_RANGE\n";
// Note: Returned NULL string
// echo gettype($r). ' val '. $r ."\n"; 


// Non-existent
echo "== NON-EXISTENT ==\n";
$r = mb_strpos($euc_jp, b'�ڹ��', 0, 'EUC-JP');
($r === FALSE) ? print "OK_STR\n"     : print "NG_STR\n";
$r = mb_strpos($euc_jp, b"\n",     0, 'EUC-JP');
($r === FALSE) ? print "OK_NEWLINE\n" : print "NG_NEWLINE\n";


// EUC-JP - No encoding parameter
echo "== NO ENCODING PARAMETER ==\n";
mb_internal_encoding('EUC-JP')  or print("mb_internal_encoding() failed\n");

print  mb_strpos($euc_jp, b'���ܸ�', 0) . "\n";
print  mb_strpos($euc_jp, b'0', 0) . "\n";
print  mb_strpos($euc_jp, 3, 0) . "\n";
print  mb_strpos($euc_jp, 0, 0) . "\n";

$r = mb_strpos($euc_jp, b'�ڹ��', 0);
($r === FALSE) ? print "OK_STR\n"     : print "NG_STR\n";
$r = mb_strpos($euc_jp, b"\n", 0);
($r === FALSE) ? print "OK_NEWLINE\n" : print "NG_NEWLINE\n";

// EUC-JP - No offset and encoding parameter
echo "== NO OFFSET AND ENCODING PARAMETER ==\n";
mb_internal_encoding('EUC-JP')  or print("mb_internal_encoding() failed\n");

print  mb_strpos($euc_jp, b'���ܸ�') . "\n";
print  mb_strpos($euc_jp, b'0') . "\n";
print  mb_strpos($euc_jp, 3) . "\n";
print  mb_strpos($euc_jp, 0) . "\n";

$r = mb_strpos($euc_jp, b'�ڹ��');
($r === FALSE) ? print "OK_STR\n"     : print "NG_STR\n";
$r = mb_strpos($euc_jp, b"\n");
($r === FALSE) ? print "OK_NEWLINE\n" : print "NG_NEWLINE\n";


// Invalid Parameters
echo "== INVALID PARAMETER TEST ==\n";

$r = mb_strpos($euc_jp,'','EUC-JP');
($r === FALSE) ? print("OK_NULL\n") : print("NG_NULL\n");
$r = mb_strpos($euc_jp, $t_ary, 'EUC-JP');
($r === FALSE) ? print("OK_ARRAY\n") : print("NG_ARRAY\n");
$r = mb_strpos($euc_jp, $t_obj, 'EUC-JP');
($r === FALSE) ? print("OK_OBJECT\n") : print("NG_OBJECT\n");
$r = mb_strpos($euc_jp, $t_obj, 'BAD_ENCODING');
($r === FALSE) ? print("OK_BAD_ENCODING\n") : print("NG_BAD_ENCODING\n");


?>

