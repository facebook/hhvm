<?php
$image = imagecreatetruecolor(180, 30);
try { $result = imagestringup($image, 1, 5, 5, 'String', 'font'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>
