<?php

touch(__DIR__.'/images/246x247.png', 1234567890);
$exif = exif_read_data(__DIR__.'/images/246x247.png');
print_r($exif);

touch(__DIR__.'/images/php.gif', 1234567890);
$exif = exif_read_data(__DIR__.'/images/php.gif');
print_r($exif);

touch(__DIR__.'/images/simpletext.jpg', 1234567890);
$exif = exif_read_data(__DIR__.'/images/simpletext.jpg');
print_r($exif);

touch(__DIR__.'/images/smile.happy.png', 1234567890);
$exif = exif_read_data(__DIR__.'/images/smile.happy.png');
print_r($exif);

touch(__DIR__.'/images/test1pix.jpg', 1234567890);
$exif = exif_read_data(__DIR__.'/images/test1pix.jpg');
print_r($exif);

touch(__DIR__.'/images/test2.jpg', 1234567890);
$exif = exif_read_data(__DIR__.'/images/test2.jpg');
print_r($exif);
