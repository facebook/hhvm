<?php
$text = "<?php echo 'test'; var_dump(__COMPILER_HALT_OFFSET__); __HALT_COMPILER(); ?>
hi there";
file_put_contents(dirname(__FILE__) . '/test1.php', $text);
$text = "<?php echo 'test2'; var_dump(__COMPILER_HALT_OFFSET__); __HALT_COMPILER(); ?>
hi there 2";
file_put_contents(dirname(__FILE__) . '/test2.php', $text);
include dirname(__FILE__) . '/test1.php';
include dirname(__FILE__) . '/test2.php';
?>
==DONE==
<?php
unlink(dirname(__FILE__) . '/test1.php');
unlink(dirname(__FILE__) . '/test2.php');
?>