<?php
$zip = zip_open(dirname(__FILE__)."/test_procedural.zip");
if (!is_resource($zip)) die("Failure");
zip_close($zip);
echo "OK";

?>
