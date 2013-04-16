<?php

touch('test/images/246x247.png', 1234567890);
$exif = exif_read_data('test/images/246x247.png');
print_r($exif);

touch('test/images/php.gif', 1234567890);
$exif = exif_read_data('test/images/php.gif');
print_r($exif);

touch('test/images/simpletext.jpg', 1234567890);
$exif = exif_read_data('test/images/simpletext.jpg');
print_r($exif);

touch('test/images/smile.happy.png', 1234567890);
$exif = exif_read_data('test/images/smile.happy.png');
print_r($exif);

touch('test/images/test1pix.jpg', 1234567890);
$exif = exif_read_data('test/images/test1pix.jpg');
print_r($exif);

touch('test/images/test2.jpg', 1234567890);
$exif = exif_read_data('test/images/test2.jpg');
print_r($exif);
