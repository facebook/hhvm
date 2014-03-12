<?php


$imagick = new Imagick(array (
            'magick:rose',
            'magick:rose',
            'magick:rose',
));

echo count ($imagick) . PHP_EOL;
echo 'done' . PHP_EOL;
?>
