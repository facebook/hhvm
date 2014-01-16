<?php
$file = b"<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";
$file .= (binary) pack(b'VVnVVV', 500, 500, 0x1000, 0x00000000, 0, 0) . (binary) str_repeat((binary)'A', 500);
file_put_contents(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php', $file);
try {
include dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
} catch (Exception $e) {
echo $e->getMessage();
}
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>