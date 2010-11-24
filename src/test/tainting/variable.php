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
 * Taint tests for functions defined in variable.idl.php
 *
 * - Please keep these tests in the same order as the function declarations in
 *   the idl files.
 * - Please make sure to test both, where a tainted result is expected, as well
 *   as when an untainted result is expected.
 */

echo "testing strval\n";
not_tainted(strval($good1));
tainted(strval($bad1));

echo "\n\n";
echo "testing var_export\n";
$a = var_export($good1, true);
not_tainted($a);

ob_start();
var_export($good1);
$a = ob_get_clean();
not_tainted($a);

$a = var_export($bad1, true);
tainted($a);

ob_start();
var_export($bad1);
$a = ob_get_clean();
tainted($a);

echo "\n\n";
echo "testing var_dump\n";
ob_start();
var_dump($good1);
$a = ob_get_clean();
not_tainted($a);

ob_start();
var_dump($bad1);
$a = ob_get_clean();
tainted($a);

echo "\n\n";
echo "testing debug_zval_dump\n";
ob_start();
debug_zval_dump($good1);
$a = ob_get_clean();
not_tainted($a);

ob_start();
debug_zval_dump($bad1);
$a = ob_get_clean();
tainted($a);

echo "\n\n";
echo "testing serialize\n";
not_tainted(serialize($good1));
tainted(serialize($bad1));

echo "\n\n";
echo "testing unserialize\n";
not_tainted(unserialize($serialized_good));
tainted(unserialize($serialized_bad));

echo "\n\n";
echo "testing get_defined_vars\n";
$arr = get_defined_vars();
not_tainted($arr['good1']);
tainted($arr['bad1']);

// Note: import_request_variables is not supported in hphp

echo "\n\n";
echo "testing extract\n";
$arr = array(
  'good1' => $good1,
  'bad1' => $bad1,
);
extract($arr, EXTR_PREFIX_ALL, 'extract');
not_tainted($extract_good1);
tainted($extract_bad1);
