<?php
$image = imagecreatetruecolor(180, 30);
try { $result = imagecharup($image, 'string', 5, 5, 'C', 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>
