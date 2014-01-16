<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

$files = array();
$files['a'] = 'abcdefg';

include 'files/phar_test.inc';

include $fname;

$fp = fopen('phar://hio/a', 'r');
var_dump(ftell($fp));
echo 'fseek($fp, 1)';var_dump(fseek($fp, 1));
var_dump(ftell($fp));
echo 'fseek($fp, 1, SEEK_CUR)';var_dump(fseek($fp, 1, SEEK_CUR));
var_dump(ftell($fp));
echo 'fseek($fp, -1, SEEK_CUR)';var_dump(fseek($fp, -1, SEEK_CUR));
var_dump(ftell($fp));
echo 'fseek($fp, -1, SEEK_END)';var_dump(fseek($fp, -1, SEEK_END));
var_dump(ftell($fp));
echo 'fseek($fp, -8, SEEK_END)';var_dump(fseek($fp, -8, SEEK_END));
var_dump(ftell($fp));
echo 'fseek($fp, -7, SEEK_END)';var_dump(fseek($fp, -7, SEEK_END));
var_dump(ftell($fp));
echo 'fseek($fp, 0, SEEK_END)';var_dump(fseek($fp, 0, SEEK_END));
var_dump(ftell($fp));
echo 'fseek($fp, 1, SEEK_END)';var_dump(fseek($fp, 1, SEEK_END));
var_dump(ftell($fp));
echo 'fseek($fp, -8, SEEK_END)';var_dump(fseek($fp, -8, SEEK_END));
var_dump(ftell($fp));
echo 'fseek($fp, 6)';var_dump(fseek($fp, 6));
var_dump(ftell($fp));
echo 'fseek($fp, 8)';var_dump(fseek($fp, 8));
var_dump(ftell($fp));
echo 'fseek($fp, -1)';var_dump(fseek($fp, -1));
var_dump(ftell($fp));
echo "next\n";
fseek($fp, 4);
var_dump(ftell($fp));
echo 'fseek($fp, -5, SEEK_CUR)';var_dump(fseek($fp, -5, SEEK_CUR));
var_dump(ftell($fp));
fseek($fp, 4);
var_dump(ftell($fp));
echo 'fseek($fp, 5, SEEK_CUR)';var_dump(fseek($fp, 5, SEEK_CUR));
var_dump(ftell($fp));
fseek($fp, 4);
var_dump(ftell($fp));
echo 'fseek($fp, -4, SEEK_CUR)';var_dump(fseek($fp, -4, SEEK_CUR));
var_dump(ftell($fp));
fseek($fp, 4);
var_dump(ftell($fp));
echo 'fseek($fp, 3, SEEK_CUR)';var_dump(fseek($fp, 3, SEEK_CUR));
var_dump(ftell($fp));
fclose($fp);
?>
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
exit(0);
 ?>