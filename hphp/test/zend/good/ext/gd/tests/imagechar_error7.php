<?php
$image = imagecreatetruecolor(180, 30);
try { $result = imagechar($image, 1, 5, 5, 'C', 'font'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>
