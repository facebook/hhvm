<?php
exif_read_data(
dirname(__FILE__) . "/bug48378.jpeg", 
"FILE,COMPUTED,ANY_TAG"
);
?>