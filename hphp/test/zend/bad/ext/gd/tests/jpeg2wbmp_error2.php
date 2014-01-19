<?php
$file = dirname(__FILE__) .'/simpletext.wbmp';
jpeg2wbmp('', $file, 20, 120, 8);
jpeg2wbmp(null, $file, 20, 120, 8);
jpeg2wbmp(false, $file, 20, 120, 8);
?>
<?php
unlink(dirname(__FILE__) .'/simpletext.wbmp');
?>
