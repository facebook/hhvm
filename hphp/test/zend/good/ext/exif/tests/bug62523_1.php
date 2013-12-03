<?php
echo "Test\n";
var_dump(count(exif_read_data(__DIR__."/bug62523_1.jpg")));
?>
Done