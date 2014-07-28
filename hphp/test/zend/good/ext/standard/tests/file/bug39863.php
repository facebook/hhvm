<?php

$filename = __FILE__ . chr(0). ".ridiculous";

if (file_exists($filename)) {
    echo "FAIL\n";
}
else {
    echo "PASS\n";
}
?>
===DONE===
<?php exit(0); ?>