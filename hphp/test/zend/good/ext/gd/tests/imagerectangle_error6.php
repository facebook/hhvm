<?php
// Create a image 
$image = imagecreatetruecolor( 100, 100 ); 

// Draw a rectangle
try { imagerectangle( $image, 0, 0, 50, 'wrong param', imagecolorallocate($image, 255, 255, 255) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
