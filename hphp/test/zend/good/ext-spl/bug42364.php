<?php
$it = new DirectoryIterator(dirname(__FILE__));

$count = 0;

foreach ($it as $e) {
    $count++;
    $type = gettype($e->getRealPath());
    if ($type != "string" && $type != "unicode") {
        echo $e->getFilename(), " is a ", gettype($e->getRealPath()), "\n";
    }
}

if ($count > 0) {
    echo "Found $count entries!\n";
}
?>
===DONE===