<?php
/* Prototype  : string gzinflate(string data [, int length])
 * Description: Unzip a gzip-compressed string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

include(dirname(__FILE__) . '/data.inc');

echo "*** Testing gzinflate() : error conditions ***\n";

echo "\n-- Testing gzcompress() function with Zero arguments --\n";
try { var_dump( gzinflate() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing gzcompress() function with more than expected no. of arguments --\n";
$data = 'string_val';
$length = 10;
$extra_arg = 10;
try { var_dump( gzinflate($data, $length, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing with a buffer that is too small --\n";
$short_len = strlen($data) - 1;
$compressed = gzcompress($data);

var_dump(gzinflate($compressed, $short_len));

echo "\n-- Testing with incorrect parameters --\n";

class Tester {
    function Hello() {
        echo "Hello\n"; 
    }
}

$testclass = new Tester();
try { var_dump(gzinflate($testclass)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gzinflate($data, $testclass)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>
===DONE===
