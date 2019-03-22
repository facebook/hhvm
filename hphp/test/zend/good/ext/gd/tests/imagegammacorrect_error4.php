<?php
$image = imagecreatetruecolor(180, 30);
try { $gamma = imagegammacorrect($image, 1, 'string'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

