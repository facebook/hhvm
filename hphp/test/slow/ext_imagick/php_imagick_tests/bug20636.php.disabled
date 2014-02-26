<?php
$image = new Imagick();
$image->newImage(0, 0, '#dddddd', 'png' );

try {
    $image->roundCorners(5, 5);
    echo "fail\n";
} catch (ImagickException $e) {
    echo "success\n";
}

?>
