<?php
file_put_contents(__DIR__."/bug66660.tmp.php", "<?php __CLASS__ ?>");
echo php_strip_whitespace(__DIR__."/bug66660.tmp.php");
?>
<?php unlink(__DIR__."/bug66660.tmp.php"); ?>