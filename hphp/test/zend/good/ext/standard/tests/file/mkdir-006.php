<?php
chdir(dirname(__FILE__));
$dirpath = "./tmp/foo//bar/logs";
mkdir($dirpath, 0777, true);

if (is_dir($dirpath)) {
    echo "Ok.\n";
} else {
    echo "Failed.\n";
}
rmdir("./tmp/foo/bar/logs");
rmdir("./tmp/foo/bar/");
rmdir("./tmp/foo/");
rmdir("./tmp/");
?>