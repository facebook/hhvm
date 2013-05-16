<?php
$image = imagecreatetruecolor(180, 30);
$resource = tmpfile();

imageistruecolor('string');
imageistruecolor($resource);
imageistruecolor(array());
?>