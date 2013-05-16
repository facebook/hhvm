<?php
$resource = tmpfile();

imagecolorallocatealpha($resource, 255, 255, 255, 50);
imagecolorallocatealpha('string', 255, 255, 255, 50);
imagecolorallocatealpha(array(), 255, 255, 255, 50);
imagecolorallocatealpha(null, 255, 255, 255, 50);
?>