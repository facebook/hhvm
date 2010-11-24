<?php
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
require_once('setup.inc');

/**
 * Taint tests for functions defined in string.idl.php
 *
 * - Please keep these tests in the same order as the function declarations in
 *   the idl files.
 * - Please make sure to test both, where a tainted result is expected, as well
 *   as when an untainted result is expected.
 */

echo "testing addcslashes\n";
not_tainted(addcslashes($good1, $good2));
tainted(addcslashes($bad1, $bad1));
tainted(addcslashes($good1, $bad1));

echo "\n\n";
echo "testing stripcslashes\n";
not_tainted(stripcslashes($good1));
tainted(stripcslashes($bad1));

echo "\n\n";
echo "testing addslashes\n";
not_tainted(addslashes($good1));
tainted(addslashes($bad1));

echo "\n\n";
echo "testing stripslashes\n";
not_tainted(stripslashes($good1));
tainted(stripslashes($bad1));

echo "\n\n";
echo "testing bin2hex\n";
not_tainted(bin2hex($good1));
tainted(bin2hex($bad1));

echo "\n\n";
echo "testing nl2br\n";
not_tainted(nl2br($good1));
tainted(nl2br($bad1));

echo "\n\n";
echo "testing quotemeta\n";
not_tainted(quotemeta($good1));
tainted(quotemeta($bad1));

echo "\n\n";
echo "testing str_shuffle\n";
not_tainted(str_shuffle($good1));
tainted(str_shuffle($bad1));

echo "\n\n";
echo "testing strrev\n";
not_tainted(strrev($good1));
tainted(strrev($bad1));

echo "\n\n";
echo "testing strtolower\n";
not_tainted(strtolower($good1));
tainted(strtolower($bad1));

echo "\n\n";
echo "testing strtoupper\n";
not_tainted(strtoupper($good1));
tainted(strtoupper($bad1));

echo "\n\n";
echo "testing ucfirst\n";
not_tainted(ucfirst($good1));
tainted(ucfirst($bad1));

echo "\n\n";
echo "testing ucwords\n";
not_tainted(ucwords($good1));
tainted(ucwords($bad1));

echo "\n\n";
echo "testing strip_tags\n";
not_tainted(strip_tags($good1));
tainted(strip_tags($bad1));

echo "\n\n";
echo "testing trim\n";
not_tainted(trim($good1));
tainted(trim($bad1));

echo "\n\n";
echo "testing ltrim\n";
not_tainted(ltrim($good1));
tainted(ltrim($bad1));

echo "\n\n";
echo "testing rtrim\n";
not_tainted(rtrim($good1));
tainted(rtrim($bad1));

echo "\n\n";
echo "testing chop\n";
not_tainted(chop($good1));
tainted(chop($bad1));

echo "\n\n";
echo "testing explode\n";
$arr = explode("\n", $good1);
not_tainted($arr[0]);
not_tainted($arr[1]);
$arr = explode("\n", $bad1);
tainted($arr[0]);
tainted($arr[1]);

echo "\n\n";
echo "testing implode\n";
$arr = array();
$arr[] = $good1;
$arr[] = $good2;
not_tainted(implode("\t", $arr));
$arr[] = $bad1;
tainted(implode("\t", $arr));

echo "\n\n";
echo "testing join\n";
$arr = array();
$arr[] = $good1;
$arr[] = $good2;
not_tainted(join("\t", $arr));
$arr[] = $bad1;
tainted(join("\t", $arr));

echo "\n\n";
echo "testing str_split\n";
$arr = str_split($good1, 2);
not_tainted($arr[0]);
not_tainted($arr[1]);
$arr = str_split($bad1, 2);
tainted($arr[0]);
tainted($arr[1]);

echo "\n\n";
echo "testing chunk_split\n";
not_tainted(chunk_split($good1, 3, $good2));
tainted(chunk_split($bad1, 3, $good1));
tainted(chunk_split($good1, 3, $bad1));

echo "\n\n";
echo "testing strtok\n";
not_tainted(strtok($good1, "\n"));
not_tainted(strtok("\n"));

tainted(strtok($bad1, "\n"));
tainted(strtok("\n"));

echo "\n\n";
echo "testing str_replace\n";

not_tainted(str_replace($good3, $good2, $good1));
$arr = array($good1, $good2);
$a = str_replace($good3, $good2, $arr);
not_tainted($a[0]);
not_tainted($a[1]);

tainted(str_replace($good3, $bad1, $good1));
$arr = array($bad1, $bad2);
$a = str_replace($good3, $good2, $arr);
tainted($a[0]);
tainted($a[1]);

echo "\n\n";
echo "testing str_ireplace\n";

not_tainted(str_ireplace($good3, $good2, $good1));
$arr = array($good1, $good2);
$a = str_ireplace($good3, $good2, $arr);
not_tainted($a[0]);
not_tainted($a[1]);

tainted(str_ireplace($good3, $bad1, $good1));
$arr = array($bad1, $bad2);
$a = str_ireplace($good3, $good2, $arr);
tainted($a[0]);
tainted($a[1]);

echo "\n\n";
echo "testing substr_replace\n";
not_tainted(substr_replace($good1, $good2, 0, 3));
not_tainted(substr_replace($good1, $good2, 0));
tainted(substr_replace($bad1, $good1, 0, 3));
tainted(substr_replace($bad1, $good1, 0));
tainted(substr_replace($good1, $bad1, 0, 3));
tainted(substr_replace($good1, $bad1, 0));
tainted(substr_replace($bad1, $bad2, 0, 3));
tainted(substr_replace($bad1, $bad2, 0));

echo "\n\n";
echo "testing substr\n";
not_tainted(substr($good1, 5, 5));
tainted(substr($bad1, 5, 5));

echo "\n\n";
echo "testing str_pad\n";
not_tainted(str_pad($good1, 5, $good2));
tainted(str_pad($bad1, 5, $good1));
tainted(str_pad($good1, 5, $bad1));
tainted(str_pad($bad1, 5, $bad2));

echo "\n\n";
echo "testing str_repeat\n";
not_tainted(str_repeat($good1, 5));
tainted(str_repeat($bad1, 13));

echo "\n\n";
echo "testing wordwrap\n";
not_tainted(wordwrap($good1, 5, $good2, true));
tainted(wordwrap($bad1, 5, $good1, true));
tainted(wordwrap($good1, 5, $bad1, true));
tainted(wordwrap($bad1, 5, $bad2, true));

echo "\n\n";
echo "testing html_entity_decode\n";
not_tainted(html_entity_decode($good1));
tainted(html_entity_decode($bad1));

echo "\n\n";
echo "testing htmlentities\n";
not_tainted(htmlentities($good1));
tainted(htmlentities($bad1));

echo "\n\n";
echo "testing convert_uuencode\n";
not_tainted(convert_uuencode($good1));
tainted(convert_uuencode($bad1));

echo "\n\n";
echo "testing md5\n";
// We consider md5 operation to generate dangerous output. It unlikely to be
// exploitable, but it's better for us to be safe than sorry...
not_tainted(md5($good1));
tainted(md5($bad1));



echo "\n\n";
echo "testing print_r\n";

$arr = array($good1, $good2);
$x = print_r($arr, true);
not_tainted($x);

$arr = array($good1, $bad1);
$x = print_r($arr, true);
tainted($x);
