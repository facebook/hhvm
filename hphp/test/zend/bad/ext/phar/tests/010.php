<?php
$file = b"<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

// this fails because the manifest length does not include the other 10 byte manifest data

$manifest = pack('V', 1) . 'a' . pack('VVVVVV', 0, time(), 0, crc32(b''), 0x00000000, 0);
$file .= pack('VVnVV', strlen($manifest), 1, 0x1000, 0x00000000, 3) . 'hio' . pack('V', 0) . $manifest;

file_put_contents(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php', $file);
try {
include dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
echo file_get_contents('phar://hio/a');
} catch (Exception $e) {
echo $e->getMessage();
}
?>
<?php error_reporting(0); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>