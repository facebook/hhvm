<?php
$file = dirname(__FILE__) .'/simpletext.wbmp';
png2wbmp('', $file, 20, 120, 8);
png2wbmp(null, $file, 20, 120, 8);
png2wbmp(false, $file, 20, 120, 8);
?>
<?php
unlink(dirname(__FILE__) .'/simpletext.wbmp');
?>
